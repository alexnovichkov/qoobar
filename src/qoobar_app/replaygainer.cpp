#include "replaygainer.h"

#include "qoobarglobals.h"
#include "enums.h"
#include "model.h"
#include "tagsreaderwriter.h"
#include "processlinemaker.h"
#include "application.h"
#include "platformprocess.h"

#include "taglib/mpc/mpcfile.h"
#include "taglib/mpc/mpcproperties.h"

#include <QTime>
#include <QTimer>
#include <QProcess>
#include <QTemporaryFile>
#include <QtDebug>
#include <QThread>
#include <QDir>


#ifdef WITH_DECODING
#include "scanner-common.h"
#include <sstream>

gboolean verbose = TRUE;
gboolean track = TRUE;

static void getAllData(struct filename_list_node *fln, QList<ReplayGainInfo> *list)
{
    struct file_data *fd = (struct file_data *) fln->d;
    double trueLoudness = clamp_rg(RG_REFERENCE_LEVEL - fd->loudness);

    ReplayGainInfo rg;
    std::stringstream ss;
    ss.precision(2);
    ss << std::fixed;
    ss << fd->gain_album << " dB"; rg.albumGain = QString::fromStdString(ss.str()); ss.str(std::string()); ss.clear();
    ss << trueLoudness << " dB"; rg.trackGain = QString::fromStdString(ss.str()); ss.str(std::string()); ss.clear();
    ss.precision(6);
    ss << fd->peak_album; rg.albumPeak = QString::fromStdString(ss.str());  ss.str(std::string()); ss.clear();
    ss << fd->peak; rg.trackPeak = QString::fromStdString(ss.str()); ss.str(std::string()); ss.clear();
    list->append(rg);
}
#endif

QString getArgument(int fileType, int operation)
{
    const char *argument=0;
    switch(operation) {
    case RG_SCAN_AS_FILES:
        argument=options[fileType].trackOptions;
        break;
    case RG_SCAN_AS_ALBUM:
    case RG_SCAN_AS_ALBUMS_BY_TAGS:
    case RG_SCAN_AS_ALBUMS_BY_FOLDERS:
        argument=options[fileType].albumOptions;
        break;
    }
    return QString::fromLatin1(argument);
}

QString fileTypeByFileID(int fileID)
{
    if (fileID<0 || fileID>Tag::FILES_NUM) return QString();
    return options[fileID].type;
}



ReplayGainer::ReplayGainer(Model *model, QObject *parent) :
    QObject(parent), m(model)
{

}

QList<ReplayGainInfo> ReplayGainer::scanWithDecoding(int fileType, const QVector<int> &indexes, int operation)
{
    QList<ReplayGainInfo> newRg;

#ifndef WITH_DECODING
    (void)fileType;
    (void)indexes;
    (void)operation;
#endif

#ifdef WITH_DECODING
    Q_EMIT message(MT_INFORMATION, "<font color=blue><b>"+tr("Processing %1").arg(fileTypeByFileID(fileType))+"</b></font>");
    App->processEvents();

    verbose = App->verbose;

    track = operation==RG_SCAN_AS_FILES;

    std::vector<char *> roots;
    for (QVector<int>::ConstIterator it = indexes.begin(); it != indexes.end(); ++it) {
        roots.push_back(g_strdup(m->fileAt(*it).fullFileName().toLocal8Bit().constData()));
    }
    GSList *errors = NULL, *files = NULL;
    Filetree tree = filetree_init(&roots[0], roots.size(), TRUE, FALSE, FALSE, &errors);

    g_slist_foreach(errors, filetree_print_error, &verbose);
    g_slist_foreach(errors, filetree_free_error, NULL);
    g_slist_free(errors);

    filetree_file_list(tree, &files);
    filetree_remove_common_prefix(files);


    int result = 0;

    GSList *fileIt = files;
    while (fileIt) {
        filename_list_node *node = (filename_list_node *)fileIt->data;
        init_and_get_number_of_frames(node, &result);
        if (!result)
            Q_EMIT message(MT_ERROR, QString("Could not read \"%1\"").arg(node->fr->raw));
        fileIt = g_slist_next(fileIt);
    }

    if (result) {
        GSList *iter = files;
        while(iter) {
            filename_list_node *node = (filename_list_node *)iter->data;

            Q_EMIT message(MT_INFORMATION, tr("Scanning %1 ...").arg(node->fr->raw));

            init_state_and_scan_work_item(node);
            iter = g_slist_next(iter);

            App->processEvents();
        }
        Q_EMIT message(MT_INFORMATION, tr("Done."));

        if (!track) {
            g_slist_foreach(files, (GFunc) calculate_album_gain_and_peak, NULL);
            calculate_album_gain_and_peak_last_dir();
        }
        Q_EMIT message(MT_SUCCESS, "<font color=green><b>"+fileTypeByFileID(fileType)+"</b> - "
                     +tr("Scanning was successful!")+"</font>");
    }
    else {
        Q_EMIT message(MT_ERROR, "<font color=red><b>"+fileTypeByFileID(fileType)+"</b> - "
                     +tr("There were errors during scanning.")+"</font>");
    }
    g_slist_foreach(files, (GFunc) destroy_state, NULL);


    if (result) {
        newRg.clear();
        g_slist_foreach(files, (GFunc) getAllData, &newRg);
    }

    g_slist_foreach(files, filetree_free_list_entry, NULL);
    g_slist_free(files);
    filetree_destroy(tree);
#endif
    return newRg;
}

QList<ReplayGainInfo> ReplayGainer::scanWithUtilities(int fileType, const QVector<int> &indexes, int operation)
{
    QList<ReplayGainInfo> newRg;
    if (!checkFileType(fileType)) return newRg;

    QStringList arguments;
    QString argument = getArgument(fileType, operation);
    if (!argument.isEmpty()) arguments = argument.split(".");

    arguments.append(getFileNames(indexes));

    PlatformProcess process(this);
    process.open(QIODevice::ReadWrite);
    ProcessLineMaker *lineMaker=0;
    lineMaker = new ProcessLineMaker(&process);
    connect(lineMaker,SIGNAL(receivedStdoutLines(QStringList)),SLOT(messages(QStringList)),Qt::QueuedConnection);
    connect(lineMaker,SIGNAL(receivedStderrLines(QStringList)),SLOT(messages(QStringList)),Qt::QueuedConnection);

    QEventLoop q;
    connect(&process,SIGNAL(finished(int)),&q,SLOT(quit()));
    connect(&process,SIGNAL(error(QProcess::ProcessError)),&q,SLOT(quit()));

    QString program = QString::fromLatin1(options[fileType].program);
#ifdef Q_OS_MAC
    program = QString("%1/%2").arg(ApplicationPaths::sharedPath())
            .arg(program);
#endif

    process.start(program, arguments);

    QTimer timer;
    connect(&timer, SIGNAL(timeout()), this, SIGNAL(tick()));
    connect(&process,SIGNAL(finished(int)),&timer,SLOT(stop()));
    timer.start(500);

    q.exec();
    if (lineMaker) lineMaker->flushBuffers();

    if (process.exitCode()==0) {
        Q_EMIT message(MT_SUCCESS, "<font color=green><b>"+fileTypeByFileID(fileType)+"</b> - "
                     +tr("Scanning was successful!")+"</font>");
    }
    else {
        Q_EMIT message(MT_ERROR, "<font color=red><b>"+fileTypeByFileID(fileType)+"</b> - "
                     +tr("There were errors during scanning.")+"</font>");
    }

    if (!copiedFiles.isEmpty()) {
        const int tagsCount = App->currentScheme->tagsCount();
        Q_FOREACH(int i, indexes) {
            QString fileName = m->fileAt(i).fullFileName();
            QString tempFile = copiedFiles.key(fileName);
            Tag tag(tempFile, tagsCount);
            TagsReaderWriter t(&tag);
            t.readTags(fileType==Tag::MP3_FILE ? TAG_APE : TAG_ALL);
            newRg << tag.replayGainInfo();
            QFile::remove(tempFile);
        }
    }

    return newRg;
}

QHash<int, QVector<int> > ReplayGainer::sortByFileType(int operation)
{
    QHash<int, QVector<int> > result;
    QVector<int> inds = m->selectedFilesIndexes();
    Q_FOREACH (int i, inds) {
        Tag tag = m->fileAt(i);
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

bool ReplayGainer::start(int operation)
{
    rgInfo.clear();
    for (int i=0; i<m->selectedFilesCount(); ++i) {
        rgInfo << m->fileAtSelection(i).replayGainInfo();
    }

    QHash<int,QVector<int> > sortedByFileType = sortByFileType(operation);

    if (sortedByFileType.isEmpty()) {
        Q_EMIT message(MT_INFORMATION, "<font color=green>"+tr("All files already have ReplayGain info!")+"</font>");
        return false;
    }

    init();

    QHash<int,QVector<int> >::const_iterator it = sortedByFileType.constBegin();
    while (it != sortedByFileType.constEnd()) {
        scanGroup(it.key(),it.value(), operation);
        ++it;
    }

    deinit();
    return true;
}

void ReplayGainer::scanGroup(int fileType, const QVector<int> &indexes, const int operation)
{
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
{
    QMap<QString,QVector<int> > result;
    Q_FOREACH(int i,indexes) {
        QString s;
        if (byFolder) s = m->fileAt(i).filePath();
        else s = m->fileAt(i).album();
        QVector<int> list = result.value(s);
        list.append(i);
        result.insert(s,list);
    }
    return result;
}

void ReplayGainer::removeRG(int fileType, const QVector<int> &indexes)
{
    if (fileType==Tag::MPC_FILE && App->mpcWriteRg) {
        Q_EMIT message(MT_WARNING, "<font color=green><b>"+fileTypeByFileID(fileType)+"</b> - "
                     +tr("Removing ReplayGain info from Musepack files is not supported.")
                     +"</font>");
    }
    else {
        for (int i=0; i<indexes.size(); ++i)
            rgInfo[indexes.at(i)] = ReplayGainInfo();
    }
}

bool ReplayGainer::checkFileType(int fileType)
{
    if (!options[fileType].program) {
        Q_EMIT message(MT_ERROR, fileTypeByFileID(fileType)+" - "
                     +tr("This type of files is not supported. Sorry\n"));
        return false;
    }
    QString program = QString::fromLatin1(options[fileType].program);
    if (!Qoobar::programInstalled(program,0)) {
        Q_EMIT message(MT_ERROR, tr("Cannot find %1.\nPlease install it.").arg(program));
        return false;
    }
    Q_EMIT message(MT_INFORMATION, QString("\n<b><font color=blue>%1 Invoking %2.</font></b>")
                 .arg(QTime::currentTime().toString()).arg(program));

    return true;
}

void ReplayGainer::scan(int fileType, const QVector<int> &indexes, int operation)
{
    if (fileType<0 || fileType>Tag::FILES_NUM) return;
    if (operation<=0 || operation>5) return;
    if (indexes.isEmpty()) return;

    App->processEvents();

    QList<ReplayGainInfo> newRgInfo;

#ifdef Q_OS_LINUX
    if ((fileType==Tag::MPC_FILE || fileType==Tag::FILES_NUM) && App->mpcWriteRg)
#endif
    {
        newRgInfo = scanWithUtilities(fileType, indexes, operation);
    }
#ifdef Q_OS_LINUX
    else {
        newRgInfo = scanWithDecoding(fileType, indexes, operation);
    }
#endif
    if (newRgInfo.size()==indexes.size())
        for (int i=0; i<indexes.size(); ++i)
            rgInfo[indexes.at(i)] = newRgInfo.at(i);
}

void ReplayGainer::messages(const QStringList &messages)
{
    Q_FOREACH (QString s, messages) {
#ifdef Q_OS_WIN
        s.replace("\n", "\r\n");
        s.replace("\r\n\n", "\r\n");
#endif
        Q_EMIT textRead(s);
    }
}

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QStringList ReplayGainer::getFileNames(const QVector<int> &indexes)
{
    QStringList result;
    copiedFiles.clear();
    Q_FOREACH(int i,indexes) {
        QString fileName=m->fileAt(i).fullFileName();

        QString newName = fileName;
        {
            QTemporaryFile f(QString("%1/temp.XXXXXX.%2")
                             .arg(QDir::tempPath())
                             .arg(m->fileAt(i).fileExt()));
            if (f.open()) newName = f.fileName();
        }
        if (QFile::copy(fileName,newName)) {
            copiedFiles.insert(newName,fileName);
            fileName = newName;
        }

//#ifdef Q_OS_WIN
//        else {
//            long size=0;
//            TCHAR* buffer = NULL;
//            LPCTSTR lpszPath = (wchar_t*)fileName.utf16();
//            size = GetShortPathName(lpszPath, NULL, 0);
//            if (size > 0) {
//                buffer = new TCHAR[size];
//                size = GetShortPathName(lpszPath, buffer, size);
//                if (size > 0)
//                    fileName = QString::fromUtf16((ushort*)buffer);
//                delete [] buffer;
//            }
//        }
//#endif
        result << fileName;
    }
    return result;
}

void ReplayGainer::init()
{
#ifdef WITH_DECODING
    input_init(PLUGIN_GSTREAMER);
#endif
}

void ReplayGainer::deinit()
{
#ifdef WITH_DECODING
    input_deinit();
#endif
}
