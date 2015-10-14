#ifndef AUTONUMBERPLUGIN_H
#define AUTONUMBERPLUGIN_H

#include <QDialog>

class QTreeWidget;
class QCheckBox;
class QSpinBox;
class QComboBox;
class Tag;
class QDialogButtonBox;

class AutonumberDialog : public QDialog
{
    Q_OBJECT
public:
    AutonumberDialog(const QList<Tag> &oldTags, QWidget *parent=0);
    virtual ~AutonumberDialog();
    QList<Tag> getNewTags();
private Q_SLOTS:
    void updateTrackNumbers();
    void accept();
private:
    QList<Tag> oldTags;
    QList<Tag> newTags;
    QSpinBox *startNumberEdit;
    QComboBox *formatComboBox;
    QComboBox *actionComboBox;
    QTreeWidget *tree;
    QCheckBox *addTotalTracksCheckBox;
    QCheckBox *resetFolderCheckBox;
    QCheckBox *resetAlbumCheckBox;
    QDialogButtonBox *buttonBox;

//    int startNumber;
//    int formatNumber;
//    bool addTotalTracks;
//    bool resetFolder;
//    bool resetAlbum;
//    int albumBehavior;
};

#endif // AutonumberPLUGIN_H
