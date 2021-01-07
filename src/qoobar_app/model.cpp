#include "model.h"

#include "enums.h"
#include "stringroutines.h"
#include "application.h"
#include "undoactions.h"
#include "tagsreaderwriter.h"
#include "tab.h"
#include "tagsrenderer.h"
#include <iostream>

#include <QTime>
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
    QAbstractTableModel(parent)
{DD;
    changedFileIndex  = -1;
    selectedFileIndex = -1;
    currentFileIndex = -1;
    readWorker = 0;
    readThread = 0;

    rgIcon = QIcon::fromTheme("replaygain");
    imgIcon = QIcon::fromTheme("image");
    saveIcon = QIcon::fromTheme("document-save");
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

bool Model::hasSelection() const
{DD;
    return !indexes.isEmpty();
}

QVector<int> Model::selectedFilesIndexes() const
{DD;
    return indexes;
}

bool Model::isSelectedFilesSaved() const
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
    const int t = tags.size();
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
    if (indexAtSelection < 0 || indexAtSelection >= indexes.size())
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

bool Model::isFilesSaved() const
{DD
    for (int i=0; i<tags.count(); ++i) {
        if (tags[i].wasChanged())
            return false;
    }
    return true;
}

int Model::selectedFilesCount() const
{DD;
    return indexes.size();
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
    beginInsertRows(QModelIndex(),tags.size(),tags.size()+filesToAdd.size()-1);
    tags.append(filesToAdd);
    endInsertRows();
    Q_EMIT filesAdded(filesToAdd.size(), updateSelected);
    Q_EMIT filesCountChanged(size());

    Q_FOREACH (const Tag &tag, filesToAdd) {
        if (tag.readOnly()) {
            Q_EMIT message(MT_WARNING,tr("Some read-only files were added,\n"
                                                  "all changes in them will not be saved!"));
            break;
        }
    }
}

void Model::addFile(const Tag &fileToAdd)
{DD;
    QList<Tag> filesToAdd;
    filesToAdd << fileToAdd;
    addFiles(filesToAdd, false);
}

void Model::delFiles()
{DD;
    beginResetModel();
    if (tags.size() == indexes.size()) {
        tags.clear();
    }
    else {
        for (int i = indexes.size()-1; i>=0; --i) {
            tags.removeAt(indexes.at(i));
        }
    }
    endResetModel();
    indexes.clear();
    Q_EMIT modelChanged(!isFilesSaved());
    Q_EMIT filesCountChanged(size());
    Q_EMIT selectionCleared();
}

void Model::save(bool selectedOnly)
{DD
    QStringList errors;
    Q_EMIT savingStarted();

    int savedCount=0;
    for (int i=0; i<size(); ++i) {
        if (selectedOnly && !indexes.contains(i)) continue;
        if (!tags.at(i).wasChanged()) continue;
        QString error;
        saveAt(i, &error);
        savedCount++;
        if (!error.isEmpty())
            errors << error;
        Q_EMIT savingProgressed(savedCount);
    }
    Q_EMIT savingFinished();

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
bool Model::saveAt(int row, QString *errorMsg)
{DD
    // wrong index
    if (row < 0 || row >= tags.count()) return false;

    TagsReaderWriter trw(&tags[row]);
    bool result = trw.writeTags();
    if (!result) {
        if (errorMsg) {
            QString f=tags.at(row).fullFileName();
            QFileInfo info(f);
            if (!info.exists()) *errorMsg = tr("%1 does not exist").arg(f);
            else if (!info.isFile()) *errorMsg = tr("%1 is not a file").arg(f);
            else if (!info.isWritable()) *errorMsg = tr("%1 is read-only").arg(f);
            else *errorMsg = f;
        }
    }

    Q_EMIT dataChanged(index(row,0),index(row,TAGSCOUNT+5)
                       ,QVector<int>()<<Qt::DecorationRole);
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
    std::sort(otherid.begin(), otherid.end());
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
        QString newFileName = newFileNames.at(i);
        if (oldFileName == newFileName) continue;

        const QString oldPath = oldFileName.left(oldFileName.lastIndexOf('/'));
        const QString newPath = newFileName.left(newFileName.lastIndexOf('/'));

        if (App->renameOptions.renamingOperation == 2) { /*rename folder*/
            if (QFileInfo(newFileName).exists() || QDir().rename(oldPath, newPath)) {
                tags[indexes.at(i)].setFile(newFileName);
                Q_EMIT dataChanged(index(i,COL_FILENAME),index(i,COL_FILENAME));
                Q_EMIT fileNameChanged(indexes.at(i), tags.at(indexes.at(i)).fileNameExt());

                // rename all other files that have the same old path
                for (int ind = 0; ind<tags.size(); ++ind) {
                    if (!indexes.contains(ind)) {
                        const QString s = tags[ind].filePath();

                        if (s == oldPath) {
                            tags[ind].setFile(newPath+"/"+tags[ind].fileNameExt());
                            Q_EMIT dataChanged(index(ind, COL_FILENAME),index(ind,COL_FILENAME));
                            Q_EMIT fileNameChanged(ind, tags.at(ind).fileNameExt());
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

            //check if the file newFileName already exists
            QFileInfo fi= QFileInfo(newFileName);
            if (fi.exists()) {
                int index=1;
                QString justName = fi.completeBaseName();
                QString justSuffix = fi.suffix();
                while (QFileInfo(QString("%1/%2 (%3).%4").arg(newPath).arg(justName).arg(index).arg(justSuffix)).exists())
                    index++;
                newFileName = QString("%1/%2 (%3).%4").arg(newPath).arg(justName).arg(index).arg(justSuffix);
            }
            if (created)
                processed = (App->renameOptions.renamingOperation != 1 ? QFile::rename(oldFileName, newFileName)
                                                                       : QFile::copy(oldFileName, newFileName));

            if (!created || !processed)
                notRenamedFiles.append(oldFileName);
            else {
                tags[indexes.at(i)].setFile(newFileName);
                Q_EMIT dataChanged(index(indexes.at(i),COL_FILENAME),index(indexes.at(i),COL_FILENAME));
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

        Q_EMIT dataChanged(index(ind,COL_SAVEICON), index(ind, COL_SAVEICON)
                           , QVector<int>()<<Qt::DecorationRole);
    }
    Q_EMIT dataChanged(index(ind,COL_IMAGE), index(ind, COL_IMAGE)
                       , QVector<int>()<<Qt::DecorationRole);
    return res;
}

void Model::setOldImage(int ind,const CoverImage &image, bool status)
{DD
    bool res=tags[ind].wasChanged();
    tags[ind].setImage(image);
    tags[ind].setChanged(status);
    if (res!=status) {
        Q_EMIT dataChanged(index(ind,COL_SAVEICON), index(ind, COL_SAVEICON)
                           , QVector<int>()<<Qt::DecorationRole);
    }
    Q_EMIT dataChanged(index(ind,COL_IMAGE), index(ind, COL_IMAGE)
                       , QVector<int>()<<Qt::DecorationRole);
}

void Model::setCurrentIndex(int current)
{
    if (current == currentFileIndex) return;
    const int oldCurrent = currentFileIndex;
    currentFileIndex = current;
    if (oldCurrent!=-1)
        Q_EMIT dataChanged(index(oldCurrent,0),index(oldCurrent,TAGSCOUNT+5)
                           ,QVector<int>()<<Qt::FontRole);
    if (currentFileIndex!=-1)
        Q_EMIT dataChanged(index(currentFileIndex,0),index(currentFileIndex,TAGSCOUNT+5)
                           ,QVector<int>()<<Qt::FontRole);
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
{DD;
    QVector<int> newIndexes = computeIndexes(indexes, up, tags.size());
    if (indexes==newIndexes) return;

    //up
    if (up) {
        for (int I=0; I<indexes.size(); ++I) {
            if (indexes.at(I)==newIndexes.at(I)) continue;
            beginMoveRows(QModelIndex(),indexes.at(I),indexes.at(I),QModelIndex(),newIndexes.at(I));
            tags.move(indexes.at(I),newIndexes.at(I));
            endMoveRows();
        }
    }
    else {//down
        for (int I=indexes.size()-1; I>=0; --I) {
            if (indexes.at(I)==newIndexes.at(I)) continue;
            beginMoveRows(QModelIndex(),indexes.at(I),indexes.at(I),QModelIndex(),newIndexes.at(I)+1);
            tags.move(indexes.at(I),newIndexes.at(I));
            endMoveRows();
        }
    }

    indexes = newIndexes;
}

void Model::sortByColumn(int column, Qt::SortOrder order, int sortType)
{DD;
    QVector<int> newIndexes;
    if (sortType==SortTime) {
        QMultiMap<int, int> map;
        for (int i=0; i<size(); ++i)
            map.insert(tags.at(i).length(),i);
        newIndexes = map.values().toVector();
    }
    else {
        bool allNumbers = true;
        for (int i=0; i<size(); ++i) {
            bool ok;
            QString s = data(index(i,column)).toString();
            int intval=s.toInt(&ok);
            if (!ok && !s.isEmpty()) {
                allNumbers = false;
                break;
            }
        }

        if (allNumbers) {
            QMultiMap<int, int> map;
            for (int i=0; i<size(); ++i) {
                QString s = data(index(i,column)).toString();
                int intval=s.toInt();
                map.insert(intval,i);
            }
            newIndexes = map.values().toVector();
        }
        else {
            QMultiMap<QString, int> map;
            for (int i=0; i<size(); ++i) {
                QString s = data(index(i,column)).toString();
                if (App->sortOption == 0)
                    map.insert(s,i);
                else
                    map.insert(s.toLower(),i);
            }
            newIndexes = map.values().toVector();
        }
    }
    if (order==Qt::DescendingOrder) std::reverse(newIndexes.begin(), newIndexes.end());

    for (int i=0; i<newIndexes.size()-1; ++i) {
        if (newIndexes.at(i) == i) continue;
        beginMoveRows(QModelIndex(),newIndexes.at(i),newIndexes.at(i),QModelIndex(),
                      newIndexes.at(i)>i?i:i+1);
        tags.move(newIndexes.at(i),i);
        endMoveRows();
        for(int j=i+1; j<newIndexes.size(); ++j) {
            if (newIndexes.at(j)<newIndexes.at(i)) newIndexes[j]+=1;
        }
    }
}

void Model::rereadTags(const QVector<int> &inds)
{DD
    Q_FOREACH (const int index, inds) {
        rereadTagsAt(index);
    }
}

void Model::rereadTagsAt(int ind)
{DD
    if (ind >= 0 && ind < size()) {
        Tag tag(tags.at(ind).fullFileName(), App->currentScheme->tagsCount());
        TagsReaderWriter t(&tag);
        t.readTags();
        if (tags.at(ind) != tag) {
            setTag(ind, tag);
        }
    }
}

void Model::setTags(const QVector<int> &inds, const QList<Tag> &newTags)
{DD
    Q_ASSERT(inds.size() == newTags.size());
    for (int i=0; i<inds.size(); ++i) {
        setTag(inds.at(i), newTags.at(i));
    }
    Q_EMIT modelChanged(!isFilesSaved());
}

void Model::setRow(int tagID, const QString &newValue)
{DD
    QStringList newValues = tagsByPattern(tagID, newValue);
    for (int i=0; i<indexes.size(); ++i) {
        setTagValue(indexes.at(i), tagID, newValues.at(i));
    }
}

void Model::setRow(const QString &stringID, const QString &newValue)
{DD
    QStringList newValues = tagsByPattern(-1, newValue);
    for (int i=0; i<indexes.size(); ++i) {
        setTagValue(indexes.at(i), stringID, newValues.at(i));
    }
}

void Model::setTagValue(int index, int tagID, const QString &newValue)
{DD
    tags[index].setTag(tagID, newValue);
}

void Model::setTagValue(int index, const QString &tagID, const QString &newValue)
{DD
    tags[index].setUserTag(tagID, newValue);
}

void Model::setTag(int ind, const Tag &tag)
{DD
    tags.replace(ind,tag);
    Q_EMIT dataChanged(index(ind,0),index(ind,TAGSCOUNT+5));
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

QStringList Model::tagsByPattern(int tagID, const QString &pattern)
{DD
    if (pattern.isEmpty()) return QVector<QString>(indexes.size(), pattern).toList();

    QStringList newValues;
    bool ok;
    const int x = pattern.toInt(&ok);
    if (ok && tagID == TRACKNUMBER) {
        const bool padWithZeros = pattern.startsWith('0');
        for (int i=x; i<x+indexes.size(); ++i) {
            if (padWithZeros) {
                newValues << QString::number(i).rightJustified(pattern.length(),QChar('0'));
            }
            else newValues << QString::number(i);
        }
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


// Reimplementations of abstract methods of QAbstractItemModel

int Model::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return tags.size();
}

int Model::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return TAGSCOUNT+5;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    const int row = index.row();
    const int column = index.column();

    if (row<0 || row>=tags.size()) return QVariant();

    const Tag &tag = tags.at(row);

    if (role == Qt::DisplayRole) {
        switch (column) {
            case 0: return QVariant(); break; // save icon
            case 1: return tag.tracknumber().simplified(); break;
            case 2: return tag.fileNameExt(); break;
            case 3: return tag.composer().simplified(); break;
            case 4: return tag.album().simplified(); break;
            case 5: return tag.title().simplified(); break;
            case 6: return tag.performer().simplified(); break;
            case 7: return tag.artist().simplified(); break;
            case 8: return tag.conductor().simplified(); break;
            case 9: return tag.orchestra().simplified(); break;
            case 10: return tag.subtitle().simplified(); break;
            case 11: return tag.key().simplified(); break;
            case 12: return tag.comment().simplified(); break;
            case 13: return tag.genre().simplified(); break;
            case 14: return tag.year().simplified(); break;
            case 15: return tag.totalTracks().simplified(); break;
            case 16: return tag.albumArtist().simplified(); break;
            case 17: return tag.category().simplified(); break;
            case 18: return tag.publisher().simplified(); break;
            case 19: return tag.copyright().simplified(); break;
            case 20: return tag.mood().simplified(); break;
            case 21: return tag.tempo().simplified(); break;
            case 22: return tag.lyricist().simplified(); break;
            case 23: return tag.lyrics().simplified(); break;
            case 24: return tag.discnumber().simplified(); break;
            case 25: return tag.totaldiscs().simplified(); break;
            case 26: return tag.encodedby().simplified(); break;
            case 27: return tag.remixedby().simplified(); break;
            case 28: return tag.rating().simplified(); break;
            case 29: return tag.originalalbum().simplified(); break;
            case 30: return tag.originalartist().simplified(); break;
            case 31: return tag.originallyricist().simplified(); break;
            case 32: return tag.url().simplified(); break;
            case 33: return tag.isrc().simplified(); break;
            case 34: return Qoobar::formatLength(tag.length()); break;
            case 35: return QVariant(); // replay gain
            case 36: return QVariant(); // image
            default: return QVariant();
        }
    }
    else if (role == Qt::EditRole) {
        switch (column) {
            case 0: return QVariant(); break; // save icon
            case 1: return tag.tracknumber(); break;
            case 2: return tag.fileNameExt(); break;
            case 3: return tag.composer(); break;
            case 4: return tag.album(); break;
            case 5: return tag.title(); break;
            case 6: return tag.performer(); break;
            case 7: return tag.artist(); break;
            case 8: return tag.conductor(); break;
            case 9: return tag.orchestra(); break;
            case 10: return tag.subtitle(); break;
            case 11: return tag.key(); break;
            case 12: return tag.comment(); break;
            case 13: return tag.genre(); break;
            case 14: return tag.year(); break;
            case 15: return tag.totalTracks(); break;
            case 16: return tag.albumArtist(); break;
            case 17: return tag.category(); break;
            case 18: return tag.publisher(); break;
            case 19: return tag.copyright(); break;
            case 20: return tag.mood(); break;
            case 21: return tag.tempo(); break;
            case 22: return tag.lyricist(); break;
            case 23: return tag.lyrics(); break;
            case 24: return tag.discnumber(); break;
            case 25: return tag.totaldiscs(); break;
            case 26: return tag.encodedby(); break;
            case 27: return tag.remixedby(); break;
            case 28: return tag.rating(); break;
            case 29: return tag.originalalbum(); break;
            case 30: return tag.originalartist(); break;
            case 31: return tag.originallyricist(); break;
            case 32: return tag.url(); break;
            case 33: return tag.isrc(); break;
            case 34: return Qoobar::formatLength(tag.length()); break;
            case 35: return QVariant(); // replay gain
            case 36: return QVariant(); // image
            default: return QVariant();
        }
    }
    else if (role == Qt::DecorationRole) {
        switch (column) {
            case COL_SAVEICON: return tag.wasChanged()?saveIcon:QIcon(); break; // save icon
            case COL_FILENAME: return QIcon::fromTheme(tag.icon()); break;
            case COL_REPLAYGAIN: return tag.replayGainInfoIsEmpty() ? QIcon():rgIcon; // replay gain
            case COL_IMAGE: return tag.imageIsEmpty() ? QIcon():imgIcon; // image
            default: return QVariant();
        }
    }
    if (role == Qt::ForegroundRole) {
        switch (column) {
            case 2: return tag.readOnly()?QBrush(Qt::gray) : qApp->style()->standardPalette().brush(QPalette::WindowText); break;
            default: return qApp->style()->standardPalette().brush(QPalette::WindowText);
        }
    }
    if (role == Qt::FontRole) {
        if (row == currentFileIndex) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
    return QVariant();
}

bool Model::setData(const QModelIndex &index, const QVariant &val, int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;

    int col = index.column();
    if (col==COL_FILENAME || col==COL_IMAGE || col==COL_LENGTH
        || col==COL_REPLAYGAIN || col==COL_SAVEICON) return false;
    if (col==COL_TRACKNUMBER) col=TRACKNUMBER;
    else if (col<=14) col -= 3;
    else col -= 2;


    if (col>=TAGSCOUNT || col<0) return false;

    const int row = index.row();
    if (row<0 || row>=size()) return false;

    QString v = val.toString();
    if (v != data(index))
        Q_EMIT tagChangeRequested(v, col);

    return true;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) return QAbstractItemModel::headerData(section, orientation, role);

    if (role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QVariant(); break; // save icon
            case 1: return tr("No."); break;
            case 2: return tr("File"); break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14: return App->currentScheme->localizedFieldName[section-3]; break;

            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33: return App->currentScheme->localizedFieldName[section-2]; break;
            case 34: return tr("Length"); break;
            case 35: return tr("RG"); // replay gain
            case 36: return tr("Img"); // image
            default: return QVariant();
        }
    }
    return QVariant();
}

//bool Model::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//}

//QMap<int, QVariant> Model::itemData(const QModelIndex &index) const
//{
//}

//bool Model::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
//{
//}

//bool Model::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
//{
//}

//void Model::fetchMore(const QModelIndex &parent)
//{
//}

//bool Model::canFetchMore(const QModelIndex &parent) const
//{
//}

Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
//    Q_UNUSED(index)
    const int col = index.column();
    if (col==COL_FILENAME || col==COL_IMAGE || col==COL_LENGTH
        || col==COL_REPLAYGAIN || col==COL_SAVEICON) return QAbstractTableModel::flags(index);

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}
