#include "replaygainer.h"

#include "qoobarglobals.h"
#include "enums.h"
#include "model.h"
#include "tagsreaderwriter.h"
#include "processlinemaker.h"
#include "application.h"
#include "platformprocess.h"
#include "stringroutines.h"
#include "taglib/mpcfile.h"
#include "taglib/mpcproperties.h"
#include "ebur128.h"
#include <libswresample/swresample.h>
#include <libavformat/avformat.h>
#include "loudgain-master/src/scan.h"
#include <QTimer>
#include <QThread>

QString fileTypeByFileID(int fileID)
{DD;
    if (fileID<0 || fileID>=Tag::FILES_NUM) return QString();
    return options[fileID].type;
}

ReplayGainer::ReplayGainer(Model *model, QObject *parent) :
    QObject(parent), m(model)
{DD;

}

QList<ReplayGainInfo> ReplayGainer::scanWithDecoding(int fileType, const QVector<int> &indexes, int operation)
{DD;
    QList<ReplayGainInfo> newRg;

    if (QThread::currentThread()->isInterruptionRequested()) return newRg;

    Q_EMIT message(MT_INFORMATION, "<font color=blue><b>"+tr("Processing %1").arg(fileTypeByFileID(fileType))+"</b></font>");

    int  ebur128_v_major     = 0;
    int  ebur128_v_minor     = 0;
    int  ebur128_v_patch     = 0;
    unsigned swr_ver         = 0;
    unsigned lavf_ver        = 0;

    // libebur128 version check -- versions before 1.2.4 aren’t recommended
    ebur128_get_version(&ebur128_v_major, &ebur128_v_minor, &ebur128_v_patch);
    if (ebur128_v_major <= 1 && ebur128_v_minor <= 2 && ebur128_v_patch < 4)
        Q_EMIT message(MT_WARNING, QString("Your EBU R128 library (libebur128) is version %1.%2.%3.\n"
               "This is an old version and might cause problems.")
                       .arg(ebur128_v_major)
                       .arg(ebur128_v_minor)
                       .arg(ebur128_v_patch));
    else
        Q_EMIT message(MT_INFORMATION, QString("Your EBU R128 library (libebur128) is version %1.%2.%3")
                       .arg(ebur128_v_major)
                       .arg(ebur128_v_minor)
                       .arg(ebur128_v_patch));

    // libavformat version
    //lavf_ver = avformat_version();
    scan_get_library_version(&lavf_ver, &swr_ver);
    Q_EMIT message(MT_INFORMATION, QString("Your libavformat library is version %1.%2.%3")
                   .arg(lavf_ver>>16)
                   .arg(lavf_ver>>8&0xff)
                   .arg(lavf_ver&0xff));

    // libswresample version
    //swr_ver = swresample_version();
    Q_EMIT message(MT_INFORMATION, QString("Your libswresample library is version %1.%2.%3")
                   .arg(swr_ver>>16)
                   .arg(swr_ver>>8 & 0xff)
                   .arg(swr_ver & 0xff));

    double preGain     = 0.f;
    if (App->replaygainOptions.loudness == 1) preGain = -5;
    double maxTruePeakLevel = -1.0; // dBTP; default for -k, as per EBU Tech 3343
    bool doAlbum = operation==RG_SCAN_AS_ALBUM || operation==RG_SCAN_AS_ALBUMS_BY_FOLDERS
            || operation==RG_SCAN_AS_ALBUMS_BY_TAGS;
    bool noClip        = App->replaygainOptions.preventClipping;
//	bool lowercase      = App->replaygainOptions.tagsCase==1;
    QString unit        = "dB";
    if (App->replaygainOptions.units==1) unit = "LU";

    int result = scan_init(indexes.size());
    if (result != 0) Q_EMIT message(MT_ERROR, QString(scan_get_last_error()));

    for (int i = 0; i < indexes.size(); i++) {
        if (QThread::currentThread()->isInterruptionRequested()) return newRg;

        Q_EMIT message(MT_INFORMATION, "<font color=green><b>"+tr("Processing %1").arg(m->fileAt(indexes.at(i)).fileNameExt())
                       +"</b></font>");

        result = scan_file(m->fileAt(indexes.at(i)).fullFileName().toUtf8().data(), i);

        if (result != 0) {
            QString m;
            switch (result) {
                case SCAN_ERROR_MEMORY: m = tr("Not enough memory: "); break;
                case SCAN_ERROR_OPEN_FILE: m = tr("Could not open input: "); break;
                case SCAN_ERROR_INDEX_OUT_OF_RANGE: m = QString::number(i)+": "; break;
                case SCAN_ERROR_NO_STREAM_INFO: m = tr("Could not find stream info: "); break;
                case SCAN_ERROR_NO_STREAM: m = tr("Audio stream: "); break;
                case SCAN_ERROR_AUDIO_CODEC: m = tr("Could not open codec: "); break;
                case SCAN_ERROR_EBUR128_INIT: m = tr("EBU R128: "); break;
                case SCAN_ERROR_SWRESAMPLE: m = tr("Could not open SWResample: "); break;
            }

            Q_EMIT message(MT_ERROR, m+QString(scan_get_last_error()));
            return newRg;
        }
    }

    // check for different file (codec) types in an album and warn
    // (including Opus might mess up album gain)
    if (doAlbum) {
        if (scan_album_has_different_containers() || scan_album_has_different_codecs()) {
            Q_EMIT message(MT_WARNING, tr("You have different file types in the same album!"));
            if (scan_album_has_opus())
                Q_EMIT message(MT_ERROR, tr("Cannot calculate correct album gain when mixing Opus and non-Opus files!"));
        }
    }

    for (int i = 0; i < indexes.size(); i++) {
        if (QThread::currentThread()->isInterruptionRequested()) return newRg;

        bool willClip = false;
        double tgain = 1.0; // "gained" track peak
        double tnew;
        double tpeak = pow(10.0, maxTruePeakLevel / 20.0); // track peak limit
        double again = 1.0; // "gained" album peak
        double anew;
        double apeak = pow(10.0, maxTruePeakLevel / 20.0); // album peak limit
        bool tclip = false;
        bool aclip = false;

        scan_result *scan = scan_get_track_result(i, preGain);

        if (scan == NULL)
            continue;

        if (doAlbum)
            scan_set_album_result(scan, preGain);

        // Check if track or album will clip, and correct if so requested (-k/-K)

        // track peak after gain
        tgain = pow(10.0, scan->track_gain / 20.0) * scan->track_peak;
        tnew = tgain;
        if (doAlbum) {
            // album peak after gain
            again = pow(10.0, scan->album_gain / 20.0) * scan->album_peak;
            anew = again;
        }

        if ((tgain > tpeak) || (doAlbum && (again > apeak)))
            willClip = true;

        // printf("\ntrack: %.2f LU, peak %.6f; album: %.2f LU, peak %.6f\ntrack: %.6f, %.6f; album: %.6f, %.6f; Clip: %s\n",
        // 	scan->track_gain, scan->track_peak, scan->album_gain, scan->album_peak,
        // 	tgain, tpeak, again, apeak, will_clip ? "Yes" : "No");

        if (willClip && noClip) {
            if (tgain > tpeak) {
                // set new track peak = minimum of peak after gain and peak limit
                tnew = std::min(tgain, tpeak);
                scan->track_gain = scan->track_gain - (log10(tgain/tnew) * 20.0);
                tclip = true;
            }

            if (doAlbum && (again > apeak)) {
                anew = std::min(again, apeak);
                scan->album_gain = scan->album_gain - (log10(again/anew) * 20.0);
                aclip = true;
            }

            willClip = false;

            // printf("\nAfter clipping prevention:\ntrack: %.2f LU, peak %.6f; album: %.2f LU, peak %.6f\ntrack: %.6f, %.6f; album: %.6f, %.6f; Clip: %s\n",
            // 	scan->track_gain, scan->track_peak, scan->album_gain, scan->album_peak,
            // 	tgain, tpeak, again, apeak, will_clip ? "Yes" : "No");
        }

        ReplayGainInfo rg;
        rg.trackGain = QString("%1 %2").arg(scan->track_gain, 0, 'f', 2).arg(unit);
        if (fileType==13)
            rg.trackGain = QString::number((int) round(scan->track_gain * 256.0));
        rg.trackPeak = QString("%1").arg(scan->track_peak, 0, 'f', 6);
        if (App->replaygainOptions.mode==1) {
            rg.trackMinMax = QString("%1 %2").arg(scan->track_loudness_range, 0, 'f', 2).arg(unit);
            rg.loudness = QString("%1 LUFS").arg(scan->track_loudness,0,'f',2);
        }
        if (doAlbum) {
            rg.albumGain = QString("%1 %2").arg(scan->album_gain, 0, 'f', 2).arg(unit);
            if (fileType==13)
                rg.albumGain = QString::number((int) round(scan->album_gain * 256.0));
            rg.albumPeak = QString("%1").arg(scan->album_peak, 0, 'f', 6);
            if (App->replaygainOptions.mode==1)
                rg.albumMinMax = QString("%1 %2").arg(scan->album_loudness_range, 0, 'f', 2).arg(unit);
        }
        newRg << rg;

        //printing result
        {
            QString m = QString("<font color=green>Track: %1</font><br/>"
                           "  Loudness: %2 LUFS<br/>"
                           "  Range:    %3 %4<br/>"
                           "  Peak:     %5 (%6 dBTP)<br/>"
                           " ").arg(scan->file)
                    .arg(scan->track_loudness,8,'g',2)
                    .arg(scan->track_loudness_range,8,'g',2).arg(unit)
                    .arg(scan->track_peak,8,'g',6)
                    .arg(20.0 * log10(scan->track_peak),0,'g',2);

            if (fileType==13) {//OPUS
                // also show the Q7.8 number that goes into R128_TRACK_GAIN
                m.append(QString("  Gain:     %1 %2 (%3)%4\n")
                         .arg(scan->track_gain,8,'g',2)
                         .arg(unit)
                         .arg((int) round(scan->track_gain * 256.0))
                 .arg(tclip ? tr(" (corrected to prevent clipping)") : ""));
            } else {
                m.append(QString("  Gain:     %1 %2%3\n").arg(scan->track_gain,8,'g',2).arg(unit)
                 .arg(tclip ? tr(" (corrected to prevent clipping)") : ""));
            }
            if (willClip)
                m.append(tr("  The track will clip"));

            Q_EMIT message(MT_INFORMATION, m);

            if ((i == (indexes.size() - 1)) && doAlbum) {
                m = QString("<font color=green>Album:</font><br/>"
                               "  Loudness: %1 LUFS<br/>"
                               "  Range:    %2 %3<br/>"
                               "  Peak:     %4 (%5 dBTP)<br/>"
                               " ")
                        .arg(scan->album_loudness,8,'g',2)
                        .arg(scan->album_loudness_range,8,'g',2).arg(unit)
                        .arg(scan->album_peak,8,'g',6)
                        .arg(20.0 * log10(scan->album_peak),0,'g',2);

                if (fileType==13) {//OPUS
                    // also show the Q7.8 number that goes into R128_ALBUM_GAIN
                    m.append(QString("  Gain:     %1 %2 (%3)%4\n")
                             .arg(scan->album_gain,8,'g',2)
                             .arg(unit)
                             .arg((int) round(scan->album_gain * 256.0))
                     .arg(aclip ? tr(" (corrected to prevent clipping)") : ""));
                } else {
                    m.append(QString("  Gain:     %1 %2%3\n").arg(scan->album_gain,8,'g',2).arg(unit)
                     .arg(aclip ? tr(" (corrected to prevent clipping)") : ""));
                }
                Q_EMIT message(MT_INFORMATION, m);
            }
        }

        free(scan);
    }


    scan_deinit();
    return newRg;
}

//QList<ReplayGainInfo> ReplayGainer::scanWithUtilities(int fileType, const QVector<int> &indexes, int operation)
//{DD;
//    QList<ReplayGainInfo> newRg;
//    if (!checkFileType(fileType)) return newRg;

//    QStringList arguments;
//    QString argument = getArgument(fileType, operation);
//    if (!argument.isEmpty()) arguments = argument.split(".");

//    arguments.append(getFileNames(indexes));

//    PlatformProcess process(this);
//    process.open(QIODevice::ReadWrite);
//    ProcessLineMaker *lineMaker = new ProcessLineMaker(&process);
//    connect(lineMaker,SIGNAL(receivedStdoutLines(QStringList)),SLOT(messages(QStringList)),Qt::QueuedConnection);
//    //connect(lineMaker,SIGNAL(receivedStderrLines(QStringList)),SLOT(messages(QStringList)),Qt::QueuedConnection);

//    QEventLoop q;
//    connect(&process,SIGNAL(finished(int)),&q,SLOT(quit()));
//    connect(&process,SIGNAL(error(QProcess::ProcessError)),&q,SLOT(quit()));

//    QString program = QString::fromLatin1(options[fileType].program);
//#ifdef Q_OS_MACOS
//    program = QString("%1/%2").arg(ApplicationPaths::sharedPath())
//            .arg(program);
//#endif

//    process.start(program, arguments);

//    QTimer timer;
//    connect(&timer, SIGNAL(timeout()), this, SIGNAL(tick()));
//    connect(&process,SIGNAL(finished(int)),&timer,SLOT(stop()));
//    timer.start(500);

//    q.exec();
//    if (lineMaker) lineMaker->flushBuffers();

//    if (process.exitCode()==0) {
//        Q_EMIT message(MT_SUCCESS, "<font color=green><b>"+fileTypeByFileID(fileType)+"</b> - "
//                     +tr("Scanning was successful!")+"</font>");
//    }
//    else {
//        Q_EMIT message(MT_ERROR, "<font color=red><b>"+fileTypeByFileID(fileType)+"</b> - "
//                     +tr("There were errors during scanning.")+"</font>");
//    }

//    if (!copiedFiles.isEmpty()) {
//        const int tagsCount = App->currentScheme->tagsCount();
//        Q_FOREACH(int i, indexes) {
//            QString fileName = m->fileAtSelection(i).fullFileName();
//            QString tempFile = copiedFiles.key(fileName);
//            Tag tag(tempFile, tagsCount);
//            TagsReaderWriter t(&tag);
//            t.readTags(fileType==Tag::MP3_FILE ? TAG_APE : TAG_ALL);
//            newRg << tag.replayGainInfo();
//            QFile::remove(tempFile);
//        }
//    }

//    return newRg;
//}

QHash<int, QVector<int> > ReplayGainer::sortByFileType(int operation)
{DD;
    QHash<int, QVector<int> > result;
    int selected = m->selectedFilesCount();
    //QVector<int> inds = m->selectedFilesIndexes();
    for (int i=0; i<selected; ++i) {
        Tag tag = m->fileAtSelection(i);
        if (!tag.replayGainInfoIsEmpty() && skip && operation != RG_SCAN_REMOVE)
            continue;
        int type=tag.fileType();
        if (type == Tag::MPC_FILE) {
            //checking version of mpc file
            TagLib::MPC::File *f = new TagLib::MPC::File(FILE_NAME(tag.fullFileName()));
            if (f) {
                TagLib::MPC::Properties *p = f->audioProperties();
                if (p) {
                    if (p->mpcVersion()<8) type=14; //hardcoded but works
                }
            }
            delete f;
        }
        QVector<int> indexes = result.value(type);
        indexes.append(i);
        result.insert(type,indexes);
    }
    return result;
}

void ReplayGainer::start()
{DD;
    rgInfo.clear();
    for (int i=0; i<m->selectedFilesCount(); ++i) {
        rgInfo << m->fileAtSelection(i).replayGainInfo();
    }

    QHash<int,QVector<int> > sortedByFileType = sortByFileType(operation);

    if (sortedByFileType.isEmpty()) {
        Q_EMIT message(MT_INFORMATION, "<font color=green>"+tr("All files already have ReplayGain info!")+"</font>");
        return;
    }

    QHash<int,QVector<int> >::const_iterator it = sortedByFileType.constBegin();
    while (it != sortedByFileType.constEnd()) {
        if (QThread::currentThread()->isInterruptionRequested()) return;
        scanGroup(it.key(),it.value(), operation);
        ++it;
    }
    Q_EMIT finished();
}

void ReplayGainer::scanGroup(int fileType, const QVector<int> &indexes, const int operation)
{DD;
    switch (operation) {
    case RG_SCAN_AS_FILES:
    case RG_SCAN_AS_ALBUM:
        scan(fileType,indexes,operation);
        break;
    case RG_SCAN_AS_ALBUMS_BY_FOLDERS:
    case RG_SCAN_AS_ALBUMS_BY_TAGS: {
        bool byFolder=(operation==RG_SCAN_AS_ALBUMS_BY_FOLDERS);
        QMap<QString,QVector<int> > map = sort(indexes,byFolder);
        QMap<QString,QVector<int> >::const_iterator it = map.constBegin();
        while (it != map.constEnd()) {
            scan(fileType, it.value(), operation);
            ++it;
        }
        break;
    }
    case RG_SCAN_REMOVE:
        removeRG(fileType, indexes);
        break;
    }
}

QMap<QString, QVector<int> > ReplayGainer::sort(const QVector<int> &indexes, bool byFolder)
{DD;
    QMap<QString,QVector<int> > result;
    Q_FOREACH(int i,indexes) {
        QString s;
        if (byFolder) s = m->fileAtSelection(i).filePath();
        else s = m->fileAtSelection(i).album();
        QVector<int> list = result.value(s);
        list.append(i);
        result.insert(s,list);
    }
    return result;
}

void ReplayGainer::removeRG(int fileType, const QVector<int> &indexes)
{DD;
    if (fileType==Tag::MPC_FILE && App->mpcWriteRg) {
        Q_EMIT message(MT_WARNING, "<font color=green><b>MPC</b> - "
                     +tr("Removing ReplayGain info from Musepack files is not supported.")
                     +"</font>");
    }
    else {
        for (int i=0; i<indexes.size(); ++i)
            rgInfo[indexes.at(i)] = ReplayGainInfo();
    }
}

void ReplayGainer::scan(int fileType, const QVector<int> &indexes, int operation)
{DD;
    if (fileType<0 || fileType>Tag::FILES_NUM) return;
    if (operation<RG_SCAN_AS_FILES || operation>=RG_SCAN_TOTAL) return;
    if (indexes.isEmpty()) return;

    App->processEvents();

    QList<ReplayGainInfo> newRgInfo = scanWithDecoding(fileType, indexes, operation);
    if (newRgInfo.size()==indexes.size()) {
        for (int i=0; i<indexes.size(); ++i)
            rgInfo[indexes.at(i)] = newRgInfo.at(i);
    }
}

void ReplayGainer::messages(const QStringList &messages)
{DD;
    Q_FOREACH (QString s, messages) {
#ifdef Q_OS_WIN
        s.replace("\n", "\r\n");
        s.replace("\r\n\n", "\r\n");
#endif
        Q_EMIT textRead(s);
    }
}


