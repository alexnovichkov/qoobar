#ifndef MP3TAGSDIALOG_H
#define MP3TAGSDIALOG_H

#include <QDialog>

class QCheckBox;

class MP3TagsDialog : public QDialog
{
    Q_OBJECT
public:
    enum MP3TAGTYPES {
        ID3V1=1,
        ID3V2=2,
        APE=4
    };

    explicit MP3TagsDialog(int fileCount, QWidget *parent = 0);
    int tagTypes() {return m_tagTypes;}
public Q_SLOTS:
    void accept();
private:
    int m_tagTypes;
    QCheckBox *id3v1CheckBox;
    QCheckBox *id3v2CheckBox;
    QCheckBox *apeCheckBox;
};

#endif // MP3TAGSDIALOG_H
