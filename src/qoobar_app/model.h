#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QFont>
#include <QIcon>

#include "tagger.h"

//columns numbers
#define COL_SAVEICON 0
#define COL_TRACKNUMBER 1
#define COL_FILENAME 2
#define COL_LENGTH 34
#define COL_REPLAYGAIN 35
#define COL_IMAGE 36

class ReadWorker : public QObject
{
    Q_OBJECT
public:
    ReadWorker(QObject *parent = 0);

    QList<Tag> tags();

    void setFiles(const QStringList &filesToAdd);

public Q_SLOTS:
    void run();

Q_SIGNALS:
    void finished();
    void fileAdded(int);
private:
    QStringList files;
    QList<Tag> _tags;
};

class QThread;
#include <QAbstractTableModel>

class Model : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = 0);

    virtual ~Model();

    /* Whole model */
    /*!
     * \brief size
     * \return number of files in a model, a whole model size
     */
    int size() const;
    /*!
     * \brief isEmpty
     * \return true if size()==0, false otherwise
     */
    bool isEmpty() const;
    /*!
     * \brief totalLength
     * \return total length of all files in seconds
     */
    int totalLength() const; //
    /*!
     * \brief filesNames
     * \return files full names
     */
    QStringList filesNames() const;
    /*!
     * \brief isSaved
     * \return true if all files are saved, see also \a selectedFilesSaved()
     */
    bool isFilesSaved() const;


    /* Selection handling */
    /*!
     * \brief selectedFilesCount
     * \return count of selected files
     */
    int selectedFilesCount() const;
    /*!
     * \brief hasSelection
     * \return true if selectedFilesCount()>0
     */
    bool hasSelection() const;
    /*!
     * \brief selectedFilesIndexes
     * \return indexes of files in selection
     */
    QVector<int> selectedFilesIndexes() const;
    /*!
     * \brief selectedFiles
     * \return files in selection
     */
    QList<Tag> selectedFiles() const;
    /*!
     * \brief selectedFilesNames
     * \return names of files in selection
     */
    QStringList selectedFilesNames() const;
    /*!
     * \brief totalLengthOfSelectedFiles
     * \return length of files in selection, in seconds
     */
    int totalLengthOfSelectedFiles() const;
    /*!
     * \brief selectedFilesSaved
     * \return true if all files in selection are saved
     */
    bool isSelectedFilesSaved() const;
    /*!
     * \brief indexAtIndexInSelection
     * \param indexAtSelection index of file in selection
     * \return index of file in model
     * f.e. if selected indexes are 0,1,3,6 then indexAtIndexInSelection(2) returns 3,
     * and indexAtIndexInSelection(3) returns 6.
     */
    int indexAtIndexInSelection(const int indexAtSelection) const;
    /*!
     * \brief fileAt
     * \param index - index in model
     * \return reference to a file at \e index in model
     */
    Tag &fileAt(int index);
    /*!
     * \brief fileAtSelection
     * \param index - index in selection
     * \return reference to a file at index in selection.
     * Equals to fileAt(indexAtIndexInSelection(index))
     */
    Tag &fileAtSelection(int index); // returns file at index from selectedFilesIndexes()




    /* Changed files handling*/
    /*!
     * \brief changedFilesCount
     * \return count of changed files regardless of whether they are selected
     */
    int changedFilesCount() const;
    /*!
     * \brief firstChangedFileIndex
     * \return index of first changed file for iterating
     * returns -1 if no changed files found
     */
    int firstChangedFileIndex();
    /*!
     * \brief nextChangedFileIndex
     * \return index of the next changed file or -1 if no changed files left
     */
    int nextChangedFileIndex();

    /*!
     * \brief imagesAreSame
     * \return true if all files in selection contain the same image or no image at all
     */
    bool imagesAreSame();


    /*!
     * \brief saveAt saves file at index
     * \param row index of file
     * \param msg string to write an error message
     * \return whether the file was successfully saved
     */
    bool saveAt(int row, QString *msg=0);

    void forceSelectionToSave();
    void move(bool up);
    void setTags(const QVector<int> &inds,const QList<Tag> &newTags);
    void setRow(int tagID, const QString &newValue);
    void setRow(const QString &stringID, const QString &newValue);
    void setTag(int ind, const Tag &tag);
    void removeAllTags();
    void removeUserTags();
    void removeImage();

    /**
     * @brief tagsByPattern
     * @param tagID
     * @param pattern
     * @return a list of strings generated by @param pattern for @param tagID
     */
    QStringList tagsByPattern(int tagID, const QString &pattern);

    void rereadTags(const QVector<int> &inds);
    void rereadTagsAt(int ind);

    QStringList values(int tagID) const;
    QStringList values(const QString &tagID) const;
    QString value(int index, int tagID) const;
    QString value(int index, const QString &tagID) const;


    QStringList userTagsIds() const;

    /**
     * Renames selection with newFileNames and returns not renamed files names
     * message will be updated accordingly
     */
    void rename(const QStringList &newFileNames);

    void setImage(const CoverImage &img);
    void setImageDescription(const QString &description);
    void setImageType(int type);
    bool writeImage(QString dirName);

    bool setNewImage(int ind,const CoverImage &image);
    void setOldImage(int ind,const CoverImage &image, bool status);

    void sortByColumn(int column, Qt::SortOrder order, int sortType);

    /**
     * @brief setCurrentIndex sets index of a file currently selected in the Tags editor
     * @param current
     */
    void setCurrentIndex(int current);
public Q_SLOTS:
    void selectAll();
    void setSelection(const QVector<int> &selectedFilesIndexes);
    /*!
     * \brief addFiles - adds files to model, emits
     * filesAdded() and filesCountChanged() signals
     * \param filesToAdd
     */
    void addFiles(const QList<Tag> &filesToAdd, bool updateSelected = true);

    void addFile(const Tag &fileToAdd);
    /**
      Reads filesToAdd in a separate thread and invokes addFiles(const QList<Tag> &filesToAdd);
    */
    void addFiles(const QStringList &filesToAdd);

    void delFiles();
    /**
     * @brief save - saves all changed files
     */
    void save(bool selectedOnly = false);

private Q_SLOTS:
    /*!
     * \brief addFiles - takes list of Tag from readWorker and appends it to model
     */
    void addFiles();



Q_SIGNALS:
    void fileChanged(int, bool); //emits when tag save state changed
    void modelChanged(bool); // emits when tag save state changed

    void fileChanged(int, const Tag&, const QVector<int>&); //emits when whole file needs updating
    void fileNameChanged(int, const QString &newFileName);
    void message(int type, const QString &text);
    void filesAdded(int,bool);
    void tagChangeRequested(const QString &newValue, int tagID);

    /**
     * @brief fileAdded emitted when file with number seqNumber was added to model
     * @param seqNumber
     */
    void fileAdded(int seqNumber);

    /**
     * @brief allFilesAdded emitted when model finishes adding files
     */
    void allFilesAdded();

    /**
     * @brief fileDeleted emitted when the file with number seqNumber was deleted from model
     * @param seqNumber
     */
    void fileDeleted(int seqNumber);

    /**
     * @brief allFilesDeleted emitted when the model was completely cleared
     */
    void allFilesDeleted();

    void filesCountChanged(int); // emits when model size changed
    void selectionCleared();

    void savingStarted();
    void savingFinished();
    void savingProgressed(int);
private:
    void setTagValue(int index, int tagID, const QString &newValue);
    void setTagValue(int index, const QString &tagID, const QString &newValue);
    QVector<int> indexes;
    QList<Tag> tags;
    int changedFileIndex;
    int selectedFileIndex;
    ReadWorker *readWorker;
    QThread *readThread;
    int currentFileIndex;

    QIcon rgIcon;
    QIcon saveIcon;
    QIcon imgIcon;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
//    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);
//    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
//    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
//    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);
//    virtual void fetchMore(const QModelIndex &parent);
//    virtual bool canFetchMore(const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // MODEL_H
