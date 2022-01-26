/**
 * \file argsparser.cpp
 * Arguments parser and command line processor
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 10 Apr 2013
 *
 * Copyright (C) 2013  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef QOOBAR_ENABLE_CLI

#include "argsparser.h"


#include "qoobarglobals.h"
#include "model.h"
#include "enums.h"
#include "placeholders.h"
#include "application.h"
#include "replaygainer.h"
#include "tagparser.h"
#include "stringroutines.h"
#include "tagsrenderer.h"
#include "filenamerenderer.h"
#include "mainwindow.h"
#include "cuesplitter.h"
#include "clparser.h"

#include <QFile>
#include <QImage>

#include "enums.h"

const char *keys[TAGSCOUNT] = {
    "COMPOSER",
    "ALBUM",
    "TITLE",
    "PERFORMER",
    "ARTIST",
    "CONDUCTOR",
    "ORCHESTRA",
    "SUBTITLE",
    "KEY",
    "COMMENT",
    "GENRE",
    "YEAR",
    "TRACKNUMBER",
    "TOTALTRACKS",
    "ALBUMARTIST",
    "CATEGORY",
    "PUBLISHER",
    "COPYRIGHT",
    "MOOD",
    "TEMPO",
    "LYRICIST",
    "LYRICS",
    "DISCNUMBER",
    "TOTALDISCS",
    "ENCODEDBY",
    "REMIXEDBY",
    "RATING",
    "ORIGINALALBUM",
    "ORIGINALARTIST",
    "ORIGINALLYRICIST",
    "URL",
    "ISRC"
};

int getRow(const QString &key)
{DD
    QString s=key.toUpper();
    for (int i=0; i < TAGSCOUNT; ++i) {
        if (s==keys[i]) return i;
    }
    return -1;
}

const char *imageTypes[21]= {
    "OTHER",
    "ICON",
    "OTHER_ICON",
    "FRONT_COVER",
    "BACK_COVER",
    "LEAFLET",
    "MEDIA",
    "LEAD_ARTIST",
    "ARTIST",
    "CONDUCTOR",
    "BAND",
    "COMPOSER",
    "LYRICIST",
    "RECORDING_LOCATION",
    "DURING_RECORDING",
    "DURING_PERFORMANCE",
    "VIDEO",
    "BRIGHT_COLORED_FISH",
    "ILLUSTRATION",
    "BAND_LOGO",
    "PUBLISHER_LOGO"
};

CommandLineProcessor::CommandLineProcessor(QObject *parent) : QObject(parent)
{DD

}

void CommandLineProcessor::setTag(CLParser *parser, Model *model, const QString &id, int row, const QString &mess, const QString &mess1)
{DD
    QString arg;
    if (parser->isSet(id, arg)) {
        if (App->verbose) {
            appendMessage(QString("<font color=blue>Changing %1 to \"%2\":</font>").arg(mess).arg(arg));
        }
        if ((row==YEAR || row==TRACKNUMBER || row==TOTALTRACKS) && arg=="0") arg.clear();
        model->setRow(row, arg);
        if (App->verbose) {
            showMessage(MT_SUCCESS, QString("%1 changing was successful").arg(mess1));
        }
    }
}

void CommandLineProcessor::parse()
{DD
    QStringList args = qApp->arguments();

    if (args.size()<2) {
        // No arguments specified. Working in GUI mode
        new MainWindow();
        return;
    }

    CLParser parser("Qoobar, a simple tagger for classical music", QString(QOOBAR_VERSION));

    QFile schemeFile(ApplicationPaths::sharedPath()+"/args.json");
    schemeFile.open(QFile::ReadOnly);
    QByteArray scheme = schemeFile.readAll();
    if (!parser.setArgScheme(scheme)) {
        showMessage(MT_ERROR, "Unable to read options scheme from args.json! Working in GUI mode.");
        App->recursive = true;
        new MainWindow();
        return;
    }

    if (!parser.parse(args)) {
        showMessage(MT_ERROR, "Unable to parse command line arguments! Working in GUI mode.");
        App->recursive = true;
        new MainWindow();
        return;
    }

    QString arg;
    if (parser.isSet("help")) {
        parser.showHelp();
        exit(0);
    }
    if (parser.isSet("version")) {
        parser.showVersion();
        exit(0);
    }
    if (parser.isSet("list-keys")) {
        std::cout << "Available keys for --set and --remove:" << std::endl;
        for (int i=0; i< TAGSCOUNT; ++i)
            std::cout << "    " << keys[i] << std::endl;
        std::cout << std::endl;
        exit(0);
    }
    if (parser.isSet("list-image-types")) {
        std::cout << "Available image types for --set-image-type:" << std::endl;
        for (int i=0; i<21; ++i)
            std::cout << "    " << imageTypes[i] << std::endl;
        std::cout << std::endl;
        exit(0);
    }
    if (parser.isSet("list-placeholders")) {
        std::cout << "Available placeholders for --fill and --rename:" << std::endl;
        for (int i = 0; i < Placeholders::placeholdersCount; ++i) {
            std::cout <<"    "<<Placeholders::placeholders[i].text
                     << "    " << Placeholders::placeholders[i].description
                     << std::endl;
        }
        std::cout  << std::endl;
        exit(0);
    }

    //extract file names to process
    QStringList filesNames = parser.option("files")->value.toStringList();
    App->recursive = parser.isSet("recursive");
    App->verbose = parser.isSet("verbose");
    App->consoleMode = parser.isCommandLineMode();
    files = Qoobar::expandFilesNames(filesNames, App->recursive);
    App->setFilesNames(files);

    bool forceGui = parser.isSet("force-gui");

    //First set all options for this session
    //rename options
    if (App->verbose) appendMessage("<font color=blue>Applying options...</font>");
    if (parser.isSet("maximum-length",arg)) {
        uint option = arg.toUInt();
        if (option==0) App->renameOptions.trimFileLength = false;
        else {
            App->renameOptions.trimFileLength = true;
            App->renameOptions.maximumLength = option;
        }
        if (App->verbose) showMessage(MT_SUCCESS, "Maximum file name length is set to "+QString::number(option));
    }
    if (parser.isSet("change-case",arg)) {
        App->renameOptions.ccase = Case::caseByValue(arg);
        App->renameOptions.changeCase = App->renameOptions.ccase!=Case::DontChange;

        if (App->verbose) {
            showMessage(MT_SUCCESS, "Change case option is set to \""+arg+"\"");
        }
    }
    setBool(App->renameOptions.replaceWinChars, "replace-win-chars", "Windows chars replacement for --rename operation is switched ", &parser);
    setBool(App->renameOptions.replaceSpaces, "replace-spaces", "Spaces replacement for --rename operation is switched ", &parser);
    setBool(App->renameOptions.removeDiacritics, "remove-diacritics", "Diacritics removing for --rename operation is switched ", &parser);
    setBool(App->renameOptions.applyToFolders, "apply-to-folders", "apply-to-folders option for --rename operation is switched ", &parser);
    if (parser.isSet("output-dir",arg)) {
        App->renameOptions.destinationFolder = arg;
        if (App->verbose)
            showMessage(MT_SUCCESS, "The destination directory for --rename operation is set to \""+arg+"\"");
    }

    //global options
    QString allowedId3Options = QSL("update-always|update-existing|delete");
    if (parser.isSet("delete-id3v1",arg)) {
        int index = allowedId3Options.indexOf(arg);
        if (index>=0) App->id3v1Synchro = index;
        if (App->verbose) showMessage(MT_SUCCESS, "Deleting of id3v1 tags is set to \""+arg+"\"");
    }
    setBool(App->trim, "simplify-whitespaces", "Tags simplifying is switched ", &parser);
    setBool(App->mp3readid3, "read-id3v2", "Reading of id3v2 tags in mp3 files is switched ", &parser);
    setBool(App->mp3readape, "read-ape", "Reading of APE tags in mp3 files is switched ", &parser);
    setBool(App->mp3writeid3, "write-id3v2", "Writing of id3v2 tags in mp3 files is switched ", &parser);
    setBool(App->mp3writeape, "write-ape", "Writing of APE tags in mp3 files is switched ", &parser);
    setBool(App->id3v1Transliterate, "id3v1-transliterate", "Transliteration of id3v1 tags is switched ", &parser);
    setBool(App->oggPictureNew, "ogg-image-new", "Using of new ogg image format is switched ", &parser);
    if (parser.isSet("force-id3v23",arg)) {
        App->id3v2version = arg=="on"?3:4;
        if (App->verbose) showMessage(MT_SUCCESS, "Writing of id3v2 tags to id3v2.3 format is switched "+arg);
    }
    if (parser.isSet("mpc-replaygain-format", arg)) {
        App->mpcWriteRg = arg=="header";
        if (App->verbose)
            showMessage(MT_SUCCESS, "Format of Musepack Replaygain info is set to \""+arg+"\"");
    }
    if (App->verbose) appendMessage("");

    if (!App->consoleMode) {
        new MainWindow();
        return;
    }

    // Now working in console mode
    //files can be empty, but if --split option is set,
    //we trying to add splitted files
    QString splitFormat = "flac";
    if (parser.isSet("format", arg)) {
        if (App->verbose) appendMessage("<font color=blue>Format of splitted files is set to \""+arg+"\"</font>");
        splitFormat = arg;
    }
    if (parser.isSet("split", arg)) {
        if (App->verbose) appendMessage("<font color=blue>Splitting a file by \""+arg+"\" CUE sheet ...</font>");

        if (!arg.isEmpty()) {
            CueSplitter splitter;
            connect(&splitter, SIGNAL(message(int,QString)),SLOT(showMessage(int,QString)));
            connect(&splitter, SIGNAL(textReady(QString)),SLOT(addText(QString)));
            splitter.setCueFile(arg);
            splitter.setFormat(splitFormat);
            splitter.split();
            QStringList splittedFiles = splitter.files();

            if (!splittedFiles.isEmpty()) {
                if (App->verbose)
                    showMessage(MT_SUCCESS, QString("Files splitting was successful."));
                files.append(splittedFiles);
                App->setFilesNames(files);
            }
        }
        else showMessage(MT_ERROR, QString("CUE sheet file is empty!"));
    }

    if (files.isEmpty()) {
        // Files empty in console mode
        showMessage(MT_ERROR, QString("No files to process were specified!"));

        if (forceGui) { // force-gui option is set
            new MainWindow();
            return;
        }
        else Q_EMIT finished();
    }

    Model model;
    QObject::connect(&model,SIGNAL(message(int,QString)),SLOT(showMessage(int,QString)),Qt::QueuedConnection);
    model.addFiles(files);
    model.selectAll();
    if (App->verbose) {
        printFiles(model.selectedFilesNames());
    }

    // show tags and exit
    if (parser.isSet("show-tags")) {
        if (App->verbose) {
            appendMessage("<font color=blue>Printing all tags...</font>");
        }
        QVector<int> indexes = model.selectedFilesIndexes();
        Q_FOREACH (int index, indexes) {
            Tag tag = model.fileAt(index);
            appendMessage("<font color=blue>"+tag.fullFileName()+"</font>",0,5);
            QString properties = QString("Time: %1    %2    [ %3 kbps @ %4 Hz - %5]")
                    .arg(Qoobar::formatLength(tag.length()))
                    .arg(Tag::fileTypeDescription(tag.fileType()))
                    .arg(tag.bitrate())
                    .arg(tag.sampleRate())
                    .arg(tag.channels()==1?"Mono":(tag.channels()==2?"Stereo":QString("%1 channels").arg(tag.channels())));
            appendMessage(properties,5,5);
            //standard tags
            for (int i=0; i<TAGSCOUNT; ++i) {
                QString s = tag.tag(i);
                if (!s.isEmpty()) {
                    appendMessage(Qoobar::untranslatedTagByID(i)+": "+s,5,5);
                }
            }
            //user tags
            QStringList ids=model.userTagsIds();
            Q_FOREACH (const QString &id, ids) {
                QString s = tag.userTag(id);
                if (!s.isEmpty()) {
                    appendMessage(id+": "+s,5,5);
                }
            }
            //image
            if (!tag.imageIsEmpty()) {
                QImage image;
                if (image.loadFromData(tag.image().pixmap())) {
                    QString format = QString("Image: %1 [ %2 x %3 ], %4")
                            .arg(tag.image().mimetype().section("/",1))
                            .arg(image.width())
                            .arg(image.height())
                            .arg(imageTypes[tag.image().type()]);
                    appendMessage(format,5,5);
                    if (!tag.image().description().isEmpty())
                        appendMessage("Image description: "+tag.image().description(),5,5);
                }
            }
            //replaygain
            if (!tag.replayGainInfoIsEmpty()) {
                ReplayGainInfo rg = tag.replayGainInfo();
                appendMessage("ReplayGain info:",5,0);
                if (!rg.albumGain.isEmpty()) appendMessage(QString("Album gain: %1").arg(rg.albumGain), 10, 0);
                if (!rg.albumPeak.isEmpty()) appendMessage(QString("Album peak: %1").arg(rg.albumPeak), 10, 0);
                if (!rg.albumMinMax.isEmpty()) appendMessage(QString("Album min/max: %1").arg(rg.albumMinMax), 10, 0);
                if (!rg.trackGain.isEmpty()) appendMessage(QString("Track gain: %1").arg(rg.trackGain), 10, 0);
                if (!rg.trackPeak.isEmpty()) appendMessage(QString("Track peak: %1").arg(rg.trackPeak), 10, 0);
                if (!rg.trackMinMax.isEmpty()) appendMessage(QString("Track min/max: %1").arg(rg.trackMinMax), 10, 0);
                if (!rg.loudness.isEmpty()) appendMessage(QString("Loudness: %1").arg(rg.loudness), 10, 0);
                if (!rg.undo.isEmpty()) appendMessage(QString("Undo: %1").arg(rg.undo), 10, 0);
            }
            appendMessage("");
        }
        Q_EMIT finished();
    }
    if (parser.isSet("write-image",arg)) {
        if (App->verbose) {
            appendMessage("<font color=blue>Writing images ...</font>");
        }
        bool ok=model.writeImage(arg);
        if (App->verbose && ok) {
            showMessage(MT_SUCCESS, QString("Images were successfully written."));
        }
    }
    if (parser.isSet("scan-rg",arg) || parser.isSet("write-rg",arg)) {
        bool writerg = parser.isSet("write-rg",arg);
        int type = RG_SCAN_AS_FILES;
        if (arg=="album") type=RG_SCAN_AS_ALBUM;
        else if (arg=="dir") type=RG_SCAN_AS_ALBUMS_BY_FOLDERS;
        else if (arg=="tag") type=RG_SCAN_AS_ALBUMS_BY_TAGS;
        if (App->verbose) {
            QString message="<font color=blue>Scanning ReplayGain info: ";
            switch (type) {
                case RG_SCAN_AS_FILES: message += "Scan per file track gain"; break;
                case RG_SCAN_AS_ALBUM: message += "Scan as a single album"; break;
                case RG_SCAN_AS_ALBUMS_BY_FOLDERS: message += "Scan as albums (by folder)"; break;
                case RG_SCAN_AS_ALBUMS_BY_TAGS: message += "Scan as albums (by album tag)"; break;
            }
            appendMessage(message+"</font>");
        }
        if (!model.isSelectedFilesSaved()) {
            model.save();
        }

        ReplayGainer gainer(&model);
        connect(&gainer,SIGNAL(message(int,QString)),this,SLOT(showMessage(int,QString)),Qt::QueuedConnection);
        connect(&gainer,SIGNAL(textRead(QString)),SLOT(addText(QString)));
        connect(&gainer,SIGNAL(tick()),SLOT(tick()));

        gainer.setSkip(parser.isSet("skip-scanned"));
        gainer.setOperation(type);
        gainer.start();
        QList<ReplayGainInfo> newRg = gainer.getNewRgInfo();

        //show rg
        appendMessage("\n<font color=blue>ReplayGain scanning results</font>");
        appendMessage("  Album gain  |  Track gain  |Album peak|Track peak| File name");
        for (int i=0; i<model.selectedFilesCount(); ++i) {
            appendMessage(QString(" %1 | %2 | %3 | %4 | %5").arg(newRg.at(i).albumGain)
                          .arg(newRg.at(i).trackGain)
                          .arg(newRg.at(i).albumPeak)
                          .arg(newRg.at(i).trackPeak)
                          .arg(model.fileAtSelection(i).fileNameExt()));
        }
        appendMessage(" ");

        if (writerg && !newRg.isEmpty()) {
            //write rg
            appendMessage("<font color=blue>Writing ReplayGain info ...</font>");
            for (int i=0; i<model.selectedFilesCount(); ++i) {
                Tag &tag = model.fileAtSelection(i);
                tag.setReplayGainInfo(newRg.at(i));
                tag.setChanged(true);
            }
            model.save();
            appendMessage("<font color=green>Done.</font>");
        }
    }
    if (parser.isSet("remove-rg")) {
        if (App->verbose) {
            appendMessage("<font color=blue>Removing ReplayGain info ...</font>");
        }
        if (!model.isSelectedFilesSaved())
            model.save();

        ReplayGainer gainer(&model);
        connect(&gainer,SIGNAL(message(QString)),this,SLOT(appendMessage(QString)),Qt::QueuedConnection);
        gainer.setSkip(parser.isSet("skip-scanned"));
        gainer.setOperation(RG_SCAN_REMOVE);
        gainer.start();
    }

    if (parser.isSet("remove-all")) {//removeAllTags, including standard tags, user tags, image, replaygain info
        if (App->verbose) {
            appendMessage("<font color=blue>Removing all tags ...</font>");
        }
        model.removeAllTags();
        if (App->verbose) {
            showMessage(MT_SUCCESS, QString("Tags removing was successful."));
        }
    }
    if (parser.isSet("remove-user-tags")) {//remove all user tags
        if (App->verbose) {
            appendMessage("<font color=blue>Removing all user tags ...</font>");
        }
        model.removeUserTags();
        if (App->verbose) {
            showMessage(MT_SUCCESS, QString("Tags removing was successful."));
        }
    }
    if (parser.isSet("remove")) {//remove standard tags by keys
        bool success=true;
        QStringList ids = parser.option("remove")->getValue().toStringList();
        Q_FOREACH (const QString &key, ids) {
            if (key.isEmpty()) {
                showMessage(MT_ERROR, QString("No key defined!"));
                success=false;
                continue;
            }
            int row = getRow(key);
            if (App->verbose) {
                appendMessage("<font color=blue>Removing tag with key \""+key+"\" ...</font>");
            }
            if (row>=0) model.setRow(row, "");
            else model.setRow(key,"");
        }
        if (App->verbose && success) {
            showMessage(MT_SUCCESS, QString("Tags removing was successful."));
        }
    }
    if (parser.isSet("remove-image")) {
        if (App->verbose) {
            appendMessage("<font color=blue>Removing image ...</font>");
        }
        model.removeImage();
        if (App->verbose) {
            showMessage(MT_SUCCESS, QString("Image removing was successful."));
        }
    }
    if (parser.isSet("fill",arg)) {
        QString pattern = arg;
        if (App->verbose) {
            appendMessage("<font color=blue>Filling in tags from file name by \""+pattern+"\" ...</font>");
        }
        if (!pattern.isEmpty()) {
            if (TagParser::needParsing(pattern)) {
                QStringList source=model.selectedFilesNames();
                TagParser::truncateFileNamesToShortest(pattern,source);

                QString placeholders = Placeholders::getWritablePlaceholders()
                        + Placeholders::getVoidPlaceholder();
                for (int i=0; i<model.selectedFilesCount(); ++i) {
                    Tag &tag = model.fileAtSelection(i);
                    PairList parsed = TagParser::parse(source.at(i), pattern);
                    Q_FOREACH(const StringPair &pair, parsed) {
                        int id = placeholders.indexOf(pair.first);
                        if (id!=-1) {
                            tag.setTag(id, pair.second);
                        }
                        else id = App->currentScheme->tagIDBySimplifiedName(pair.first);
                        if (id >= 0) {
                            tag.setTag(id, pair.second);
                        }
                    }
                }
                if (App->verbose) {
                    showMessage(MT_SUCCESS, QString("Filling in tags was successful."));
                }
            }
            else {
                showMessage(MT_ERROR, QString("Fill pattern does not contain any placeholders!"));
            }
        }
        else {
            showMessage(MT_ERROR, QString("Fill pattern is empty!"));
        }
    }
    if (parser.isSet("set",arg)) {
        QStringList ids = parser.option("set")->getValue().toStringList();
        bool success=true;
        Q_FOREACH (const QString &s, ids) {
            QString key = s.section(':',0,0);
            QString value = s.section(':', 1);
            if (key.isEmpty()) {
                showMessage(MT_ERROR, QString("No key defined!"));
                success=false;
                continue;
            }
            int row = getRow(key);
            if (App->verbose) {
                appendMessage("<font color=blue>Changing tags with key \""+key+"\" to \""+value+"\" ...</font>");
            }
            if (row>=0) model.setRow(row, value);
            else model.setRow(key,value);
        }
        if (App->verbose && success) {
            showMessage(MT_SUCCESS, QString("Tags changing was successful."));
        }
    }
    setTag(&parser, &model, "album", ALBUM, "album", "Album");
    setTag(&parser, &model, "composer", COMPOSER, "composer", "Composer");
    setTag(&parser, &model, "title", TITLE, "title", "Title");
    setTag(&parser, &model, "artist", ARTIST, "artist", "Artist");
    setTag(&parser, &model, "comment", COMMENT, "comment", "Comment");
    setTag(&parser, &model, "genre", GENRE, "genre", "Genre");
    setTag(&parser, &model, "year", YEAR, "year", "Year"); //
    setTag(&parser, &model, "track-number", TRACKNUMBER, "track number", "Track number");
    setTag(&parser, &model, "total-tracks", TOTALTRACKS, "total tracks", "Total tracks");

    if (parser.isSet("set-image",arg)) {
        if (App->verbose) {
            appendMessage("<font color=blue>Inserting image from \""+arg+"\" ...</font>");
        }
        if (arg.isEmpty())
            model.removeImage();
        else if (QFile::exists(arg)) {
            QFile f(arg);  f.open(QFile::ReadOnly);
            QByteArray data = f.readAll();
            QImage img;
            if (img.loadFromData(data)) {
                CoverImage cover(data,3,"image/png",f.fileName());
                if (arg.endsWith("jpg",Qt::CaseInsensitive) || arg.endsWith("jpeg",Qt::CaseInsensitive))
                    cover.setMimetype("image/jpeg");
                model.setImage(cover);
                if (App->verbose) {
                    showMessage(MT_SUCCESS, QString("Image inserting was successful."));
                }
            }
            else {
                showMessage(MT_ERROR, "Cannot read image from \""+arg+"\"!");
            }
        }
        else {
            showMessage(MT_ERROR, "No file  \""+arg+"\" exists!");
        }
    }

    if (parser.isSet("set-image-description",arg)) {
        if (App->verbose) {
            appendMessage("<font color=blue>Setting image description to \""+arg+"\" ...</font>");
        }
        model.setImageDescription(arg);
        if (App->verbose) {
            showMessage(MT_SUCCESS, QString("Changing image description was successful."));
        }
    }
    if (parser.isSet("set-image-type",arg)) {
        if (App->verbose) {
            appendMessage("<font color=blue>Setting image type to \""+arg+"\" ...</font>");
        }
        for (int i=0; i<21; ++i) {
            if (arg == imageTypes[i]) {
                model.setImageType(i);
                if (App->verbose) {
                    showMessage(MT_SUCCESS, QString("Changing image type was successful."));
                }
            }
        }
    }
    model.save();

    if (parser.isSet("rename",arg)) {
        if (App->verbose) appendMessage("<font color=blue>Renaming files with pattern \""+arg+"\" ...</font>");

        if (!arg.isEmpty()) {
            QStringList oldFileNames = model.selectedFilesNames();
            FileNameRenderer filenameRenderer(&model);
            filenameRenderer.setPattern(arg);
            QStringList newFileNames = filenameRenderer.newTags();
            if (oldFileNames != newFileNames) {
                model.rename(newFileNames);
                if (App->verbose) {
                    showMessage(MT_SUCCESS, QString("New file names are:"));
                    Q_FOREACH(QString name, newFileNames) appendMessage(name,5,0);
                }
            }
            else appendMessage("Nothing to rename, files names not changed with this pattern.");
        }
        else showMessage(MT_ERROR, QString("Rename pattern is empty!"));
        files = model.filesNames();
        App->setFilesNames(files);
    }

    if (forceGui)
        new MainWindow();
    else
        Q_EMIT finished();

}

void CommandLineProcessor::appendMessage(QString message, int indentSpaces, int secondLineOffset)
{DD
    message.replace("<font color=red>","\033[31m");
    message.replace("<font color=green>","\033[32m");
    message.replace("<font color=blue>","\033[34m");
    message.replace("</font>","\033[0m");
    message.remove("<b>");
    message.remove("</b>");
    spacePrint(std::cout, message, indentSpaces, secondLineOffset);
}

void CommandLineProcessor::showMessage(int type, const QString &text)
{DD
    switch (type) {
        case MT_ERROR: appendMessage("<font color=red>Error: </font>"+text);
            break;
        case MT_INFORMATION: appendMessage(text);
            break;
        case MT_WARNING: appendMessage("<font color=red>Warning: </font>"+text);
            break;
        case MT_SUCCESS: appendMessage("<font color=green>OK</font>, "+text);
    }
}

void CommandLineProcessor::addText(const QString &text)
{DD
    std::cout << qPrintable(text) <<std::flush;
}

void CommandLineProcessor::tick()
{DD
    //addText(".");
    std::cout << "."<<std::flush;
}

void CommandLineProcessor::printFiles(const QStringList &files)
{DD
    if (files.isEmpty()) return;
    showMessage(MT_SUCCESS, QString("%1 files were added:").arg(files.size()));
    Q_FOREACH(const QString &file, files) {
        appendMessage(file,5,5);
    }
    appendMessage("");
}

void CommandLineProcessor::setBool(bool &dest, const QString &id, const QString &message, CLParser *parser)
{DD
    QString v;
    if (parser->isSet(id, v)) {
        dest = v=="on";
        if (App->verbose) showMessage(MT_SUCCESS, message+v);
    }
}

#endif //QOOBAR_ENABLE_CLI
