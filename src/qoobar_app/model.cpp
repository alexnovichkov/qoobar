#include "model.h"

#include "enums.h"
#include "stringroutines.h"
#include "application.h"
#include "undoactions.h"
#include "tagsreaderwriter.h"
#include "tab.h"
#include "tagsrenderer.h"
#include <iostream>

#include <QThread>
#include <QtDebug>


ReadWorker::ReadWorker(QObject *parent) :
    QObject(parent)
{DD

}

QList<Tag> ReadWorker::tags()
{DD
    return _tags;
}

void ReadWorker::setFiles(const QStringList &filesToAdd)
{DD
    files = filesToAdd;
}

void ReadWorker::run()
{DD
    _tags.clear();
    int i = 0;
    const int tagsCount = App->currentScheme->tagsCount();
    Q_FOREACH (const QString &fileName, files) {
        Tag tag = Tag(fileName, tagsCount);
        TagsReaderWriter t(&tag);
        t.readTags();
        _tags << tag;
        Q_EMIT fileAdded(i++);
    }
    Q_EMIT finished();
}



Model::Model(QObject *parent) :
    QObject(parent)
{DD;
    changedFileIndex  = -1;
    selectedFileIndex = -1;
    readWorker = 0;
    readThread = 0;
}

Model::~Model()
{DD;
    delete readWorker;
    if (readThread) {
        readThread->quit();
        readThread->deleteLater();
    }
}

void Model::selectAll()
{DD
    indexes.clear();
    for (int i=0; i<tags.size(); ++i)
        indexes << i;
}

bool Model::isEmpty() const
{DD
    return tags.isEmpty();
}

int Model::changedFilesCount() const
{DD
    int count=0;
    const int si = size();
    for (int i=0; i<si; ++i) {
        if (tags.at(i).wasChanged()) count++;
    }
    return count;
}

int Model::firstChangedFileIndex()
{DD
    changedFileIndex = -1;
    return nextChangedFileIndex();
}

int Model::nextChangedFileIndex()
{DD
    changedFileIndex++;

    for (int i=changedFileIndex; i<size(); ++i) {
        if (fileAt(i).wasChanged()) {
            changedFileIndex = i;
            return i;
        }
    }
    return -1;
}

int Model::firstSelectedFileIndex()
{DD
    selectedFileIndex = -1;
    return nextSelectedFileIndex();
}

int Model::nextSelectedFileIndex()
{DD
    selectedFileIndex++;

    if (indexes.contains(selectedFileIndex)) return selectedFileIndex;

    return -1;
}

bool Model::hasSelection() const
{DD
    return !indexes.isEmpty();
}

bool Model::selectedFilesSaved() const
{DD
    for (int i=0; i<indexes.size(); ++i) {
        if (tags.at(indexes.at(i)).wasChanged())
            return false;
    }
    return true;
}

void Model::setSelection(const QVector<int> &selection)
{DD
    indexes = selection;
}

int Model::totalLength() const
{DD
    int totalLength=0;
    const int t = size();
    for (int i=0; i<t; ++i) totalLength += tags.at(i).length();
    return totalLength;
}

int Model::totalLengthOfSelectedFiles() const
{DD
    int totalLength=0;
    Q_FOREACH(int i, indexes) totalLength += tags.at(i).length();
    return totalLength;
}

int Model::indexAtIndexInSelection(const int indexAtSelection) const
{DD
    if (indexes.isEmpty())
        return -1;
    if (indexAtSelection < 0 || indexAtSelection >= selectedFilesCount())
        return -1;
    return indexes.at(indexAtSelection);
}

QList<Tag> Model::selectedFiles() const
{DD
    QList<Tag> files;
    Q_FOREACH (int i,indexes)
        files << tags.at(i);
    return files;
}

bool Model::isSaved() const
{DD
    for (int i=0; i<tags.count(); ++i) {
        if (tags[i].wasChanged())
            return false;
    }
    return true;
}

bool Model::imagesAreSame()
{DD
    if (!hasSelection()) return true;

    Tag &tag = fileAtSelection(0);
    QByteArray h = tag.image().pixmap();

    for (int i=1; i<indexes.size(); ++i) {
        QByteArray hh = fileAtSelection(i).image().pixmap();
        if (hh!=h) {
            return false;
        }
    }
    return true;
}

int Model::size() const
{DD
    return tags.size();
}

Tag& Model::fileAt(int index)
{DD
    return tags[index];
}

Tag &Model::fileAtSelection(int index)
{DD
    return tags[indexes.at(index)];
}

void Model::addFiles(const QStringList &filesToAdd)
{DD
    if (!readWorker) readWorker = new ReadWorker();
    readWorker->setFiles(filesToAdd);

    if (!readThread) {
        readThread = new QThread;
        readWorker->moveToThread(readThread);
        connect(readWorker, SIGNAL(finished()), this, SLOT(addFiles()));
        connect(readThread, SIGNAL(started()), readWorker, SLOT(run()));
        connect(readWorker, SIGNAL(finished()), readThread, SLOT(quit()));
        connect(readWorker, SIGNAL(finished()), this, SIGNAL(allFilesAdded()));
        connect(readWorker, SIGNAL(fileAdded(int)),this,SIGNAL(fileAdded(int)));
    }

    readThread->start();
}

void Model::addFiles()
{DD;
    if (readWorker) {
        addFiles(readWorker->tags());
    }
}

void Model::addFiles(const QList<Tag> &filesToAdd, bool updateSelected)
{DD;
    tags.append(filesToAdd);
    Q_EMIT filesAdded(filesToAdd, updateSelected);
    Q_EMIT filesCountChanged(size());
}

void Model::addFile(const Tag &fileToAdd)
{DD;
    QList<Tag> filesToAdd;
    filesToAdd << fileToAdd;
    addFiles(filesToAdd, false);
}

void Model::delFiles()
{DD;
    if (tags.size() == indexes.size()) {
        tags.clear();
        Q_EMIT allFilesDeleted();
    }
    else {
        for (int i = indexes.size()-1; i>=0; --i) {
            tags.removeAt(indexes.at(i));
            Q_EMIT fileDeleted(indexes.at(i));
        }
    }

    indexes.clear();
    Q_EMIT modelChanged(!isSaved());
    Q_EMIT filesCountChanged(size());
    Q_EMIT selectionCleared();
}

void Model::save()
{DD
    QStringList errors;

    for (int i=0; i<size(); ++i) {
        QString error;
        saveAt(i, &error);
        if (!error.isEmpty())
            errors << error;
    }

    if (!errors.isEmpty())
        Q_EMIT message(MT_WARNING, tr("Cannot write tags to files:\n%1")
                     .arg(errors.join(QChar('\n'))));
}

/**
 * @brief Model::saveAt - saves file at index and emits fileChanged(index, newWasChanged)
 * if file status was changed
 * @param index
 * @param errorMsg
 * @return true if tags were written
 */
bool Model::saveAt(int index, QString *errorMsg)
{DD
    // wrong index
    if (index < 0 || index >= tags.count()) return false;

    // file was not changed
    const bool oldWasChanged = tags.at(index).wasChanged();
    if (!oldWasChanged) return false;

    int count = changedFilesCount();

    TagsReaderWriter trw(&tags[index]);
    bool result = trw.writeTags();
    if (!result) {
        if (errorMsg) {
            QString f=tags.at(index).fullFileName();
            QFileInfo info(f);
            if (!info.exists()) *errorMsg = tr("%1 does not exist").arg(f);
            else if (!info.isFile()) *errorMsg = tr("%1 is not a file").arg(f);
            else if (!info.isWritable()) *errorMsg = tr("%1 is read-only").arg(f);
            else *errorMsg = f;
        }
    }

    //if new status differs from old status, emit signal to update file icon
    const bool newWasChanged = tags.at(index).wasChanged();
    if (oldWasChanged != newWasChanged)
        Q_EMIT fileChanged(index, newWasChanged);

    // emit signal to update mainwindow status
    // only if no changed files left
    if (count == 1 && !newWasChanged)
        Q_EMIT modelChanged(false);

    return result;
}

void Model::forceSelectionToSave()
{DD
    Q_FOREACH (int i, indexes)
        tags[i].setChanged(true);
}


QStringList Model::values(int tagID) const
{DD
    QStringList list;
    Q_FOREACH(const int &i, indexes)
        list << tags[i].tag(tagID);
    return list;
}

QStringList Model::values(const QString &tagID) const
{DD
    QStringList list;
    Q_FOREACH(const int &i,indexes)
        list << tags[i].userTag(tagID);
    return list;
}

QString Model::value(int index, int tagID) const
{DD
    return tags.at(index).tag(tagID);
}

QString Model::value(int index, const QString &tagID) const
{DD
    return tags.at(index).userTag(tagID);
}

QStringList Model::selectedFilesNames() const
{DD
    QStringList list;
    Q_FOREACH(const int &i,indexes)
        list << tags.at(i).fullFileName();
    return list;
}

QStringList Model::filesNames() const
{DD
    QStringList list;
    for (int i=0; i<size(); ++i)
        list << tags.at(i).fullFileName();
    return list;
}

QStringList Model::userTagsIds() const
{DD
    QStringList otherid;
    Q_FOREACH (int i, indexes)
        otherid.append(tags.at(i).userTagsKeys());
    qSort(otherid);
    otherid.removeDuplicates();
    return otherid;
}

void Model::rename(const QStringList &newFileNames)
{DD
    QStringList notRenamedFiles;
    QString messageStr;

    Q_ASSERT(newFileNames.size() == indexes.size());

    for (int i=0; i<indexes.size(); ++i) {
        const QString oldFileName = tags[indexes.at(i)].fullFileName();
        const QString newFileName = newFileNames.at(i);
        if (oldFileName == newFileName) continue;

        const QString oldPath = oldFileName.left(oldFileName.lastIndexOf('/'));
        const QString newPath = newFileName.left(newFileName.lastIndexOf('/'));

        if (App->renameOptions.renamingOperation == 2) {
            if (QFileInfo(newFileName).exists() || QDir().rename(oldPath, newPath)) {
                tags[indexes.at(i)].setFile(newFileName);
                Q_EMIT fileNameChanged(indexes.at(i), tags.at(indexes.at(i)).fileNameExt());

                for (int index = 0; index<tags.size(); ++index) {
                    if (!indexes.contains(index)) {
                    const QString s = tags[index].filePath();

                    if (s == oldPath) {
                        tags[index].setFile(newPath+"/"+tags[index].fileNameExt());
                        Q_EMIT fileNameChanged(index, tags.at(index).fileNameExt());
                    }
                    }
                }
            }
            else {
                notRenamedFiles.append(oldPath);
            }
        }
        else {
            QDir dir(newPath);
            bool created = true;
            bool processed = true;
            if (!dir.exists()) created = QDir().mkpath(newPath);

            if (created)
                processed = (App->renameOptions.renamingOperation != 1 ? QFile::rename(oldFileName, newFileName)
                                                                       : QFile::copy(oldFileName, newFileName));

            if (!created || !processed)
                notRenamedFiles.append(oldFileName);
            else {
                tags[indexes.at(i)].setFile(newFileName);
                Q_EMIT fileNameChanged(indexes.at(i), tags.at(indexes.at(i)).fileNameExt());
            }
            if (App->renameOptions.renamingOperation==0 && App->renameOptions.removeFolder) {
                QFileInfo fi(oldFileName);
                QDir().rmdir(fi.absolutePath());
            }
        }
    }

    if (!notRenamedFiles.isEmpty()) {
        if (App->renameOptions.renamingOperation==0)
            messageStr = tr("Cannot rename following files:\n");
        else if (App->renameOptions.renamingOperation==1)
            messageStr = tr("Cannot copy following files:\n");
        else if (App->renameOptions.renamingOperation==2)
            messageStr = tr("Cannot rename following folders:\n");
        Q_EMIT message(MT_WARNING, messageStr + notRenamedFiles.join(QSL("\n")));
    }
    else {
        if (App->renameOptions.renamingOperation==0)
            messageStr = tr("All files were successfully renamed");
        else if (App->renameOptions.renamingOperation==1)
            messageStr = tr("All files were successfully copied");
        else if (App->renameOptions.renamingOperation==2)
            messageStr = tr("All folders were successfully renamed");
        Q_EMIT message(MT_INFORMATION, QString("<font color=green>%1</font>").arg(messageStr));
    }
}

void Model::setImage(const CoverImage &img)
{DD
    Q_FOREACH (const int &i, indexes) {
        setNewImage(i, img);
    }
}

void Model::setImageDescription(const QString &description)
{DD
    Q_FOREACH (int i, indexes) {
        tags[i].setImageDescription(description);
    }
}

void Model::setImageType(int type)
{DD
    Q_FOREACH (int i, indexes) {
        tags[i].setImageType(type);
    }
}

bool Model::writeImage(QString dirName)
{DD
    bool dirIsEmpty = dirName.isEmpty();
    if (!dirIsEmpty) {
        if (dirName.startsWith(QLS("~"))) {
            dirName.replace(0,1,QDir::homePath());
        }

        QDir dir(dirName);
        if (!dir.exists()) {
            if (!dir.mkdir(dirName))  {
                Q_EMIT message(MT_ERROR, QString("Cannot create directory \"%1\" to write images").arg(dirName));
                return false;
            }
        }
        dirName = dir.absolutePath();
    }

    Q_FOREACH (int i, indexes) {
        CoverImage img = tags.at(i).image();
        if (img.isEmpty()) continue;

        if (dirIsEmpty) dirName = tags.at(i).filePath();
        QString fileName = tags.at(i).fileNameExt();
        QString ext = QSL(".jpg");
        if (img.mimetype()=="image/png") ext=QSL(".png");
        fileName.append(ext);
        fileName = QString("%1/%2").arg(dirName).arg(fileName);

        QImage image;
        image.loadFromData(img.pixmap());
        if (!image.save(fileName)) {
            Q_EMIT message(MT_ERROR, QString("Cannot write image into %1").arg(fileName));
        }
    }
    return true;
}

bool Model::setNewImage(int ind,const CoverImage &image)
{DD
    bool res=tags[ind].wasChanged();
    tags[ind].setImage(image);
    if (res!=tags[ind].wasChanged()) {
        Q_EMIT fileChanged(ind, tags[ind].wasChanged());
    }
    Q_EMIT imageChanged(ind, tags[ind].imageIsEmpty());
    return res;
}

void Model::setOldImage(int ind,const CoverImage &image, bool status)
{DD
    bool res=tags[ind].wasChanged();
    tags[ind].setImage(image);
    tags[ind].setChanged(status);
    if (res!=status) {
        Q_EMIT fileChanged(ind, status);
    }
    Q_EMIT imageChanged(ind, tags[ind].imageIsEmpty());
}

void Model::sort(const QVector<int> &newIndexes)
{
   for(int i=0; i<newIndexes.size(); ++i) {
       tags.move(newIndexes.at(i), i);
   }

   indexes.clear();
}

QVector<int> computeIndexes(QVector<int> notYetMoved, bool up, int totalSize)
{DD
    QVector<int> moved;
    if (up) {
        while (notYetMoved.size()>0) {
            int j=notYetMoved.first();
            notYetMoved.pop_front();
            if (j==0 || moved.contains(j-1)) moved << j;
            else moved << j-1;
        }
    }
    else {
        int lastIndex = totalSize-1;
        while (notYetMoved.size()>0) {
            int j=notYetMoved.last();
            notYetMoved.pop_back();
            if (j==lastIndex || moved.contains(j+1)) moved.prepend(j);
            else moved.prepend(j+1);
        }
    }
    return moved;
}

void Model::move(bool up)
{DD
    QVector<int> newIndexes = computeIndexes(indexes, up, tags.size());
    int i=up?0:indexes.size()-1;
    while (1) {
        tags.move(indexes.at(i),newIndexes.at(i));
        if ((up && i==indexes.size()-1) || (!up && i==0)) break;
        i=up?i+1:i-1;
    }
    indexes=newIndexes;
}

void Model::rereadTags(const QVector<int> &inds)
{DD
    Q_FOREACH (const int index, inds) {
        rereadTagsAt(index);
    }
}

void Model::rereadTagsAt(int index)
{DD
    if (index >= 0 && index < size()) {
        Tag tag(tags.at(index).fullFileName(), App->currentScheme->tagsCount());
        TagsReaderWriter t(&tag);
        t.readTags();
        if (tags.at(index) != tag)
            setTag(index, tag);
    }
}

void Model::setTags(const QVector<int> &inds, const QList<Tag> &newTags)
{DD
    Q_ASSERT(inds.size() == newTags.size());
    for (int i=0; i<inds.size(); ++i) {
        setTag(inds.at(i), newTags.at(i));
    }
    Q_EMIT modelChanged(!isSaved());
}

void Model::setRow(int tagID, const QString &newValue)
{DD
//    if (tagID>=TAGSCOUNT) return;
    QStringList newValues = tagsByPattern(tagID, newValue);
    for (int i=0; i<indexes.size(); ++i) {
        setTag(indexes.at(i), tagID, newValues.at(i));
    }
}

void Model::setRow(const QString &stringID, const QString &newValue)
{DD
    QStringList newValues = tagsByPattern(-1, newValue);
    for (int i=0; i<indexes.size(); ++i) {
        setTag(indexes.at(i), stringID, newValues.at(i));
    }
}

void Model::setTag(int index, int tagID, const QString &newValue)
{DD
    tags[index].setTag(tagID, newValue);
}

void Model::setTag(int index, const QString &tagID, const QString &newValue)
{DD
    tags[index].setUserTag(tagID, newValue);
}

void Model::setTag(int index, const Tag &tag)
{DD
    bool status = tags.at(index).wasChanged();
    bool newStatus = tag.wasChanged();
    tags.replace(index,tag);

    Q_EMIT fileChanged(index, tag, QVector<int>()); //update whole row
    if (newStatus != status)
        Q_EMIT fileChanged(index, newStatus);
}

void Model::removeAllTags()
{DD
    Q_FOREACH (int i, indexes) {
        tags[i].removeAllTags();
    }
}

void Model::removeUserTags()
{DD
    Q_FOREACH (int i, indexes) {
        tags[i].removeUserTags();
    }
}

void Model::removeImage()
{DD
    setImage(CoverImage());
}

//bool Model::newTagsEqual(const QStringList &newTags, int tagID, const QString &stringID)
//{DD
//    for (int i=0; i<newTags.size(); ++i) {
//        QString oldTag;
//        if (tagID<TAGSCOUNT)
//            oldTag = tags.at(indexes.at(i)).tag(tagID);
//        else
//            oldTag = tags.at(indexes.at(i)).userTag(stringID);
//        if (newTags.at(i) != oldTag) return false;
//    }
//    return true;
//}

QStringList Model::tagsByPattern(int tagID, const QString &pattern)
{DD
    if (pattern.isEmpty()) return QVector<QString>(indexes.size(), pattern).toList();

    QStringList newValues;
    bool ok;
    const int x = pattern.toInt(&ok);
    if (ok && tagID == TRACKNUMBER) {
        const bool n = pattern.startsWith('0');
        for (int i=x; i<x+indexes.size(); ++i)
            newValues << (n?QString::number(i).rightJustified(2,QChar('0')):QString::number(i));
    }
    else {
        if (pattern.contains('[') || pattern.contains('{') || pattern.contains('%')
            || pattern.contains('$') || pattern.contains('<')) {
            TagsRenderer renderer(this);
            renderer.setPattern(pattern);
            newValues = renderer.newTags();
        }
        else
            newValues=QVector<QString>(indexes.size(), pattern).toList();
    }
    return newValues;
}
