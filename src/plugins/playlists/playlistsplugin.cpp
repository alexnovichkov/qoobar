#include "playlistsplugin.h"

#include <QtWidgets>

#include "enums.h"
#include "AbstractPlaylist.h"

QList<Tag> PlaylistsPlugin::getNewTags(const QList<Tag> &oldTags)
{
    Dialog dialog(oldTags);
    if (dialog.exec()) {
        return dialog.getNewTags();
    }
    return oldTags;
}

Dialog::Dialog(const QList<Tag> &oldTags, QWidget *parent)
    : QDialog(parent), oldTags(oldTags)
{
    setWindowTitle(tr("Create playlist"));

#ifdef QOOBAR_PORTABLE
    QSettings se(QSL("qoobar.ini"),QSettings::IniFormat);
#else
    QSettings se("qoobar","qoobar");
#endif
    se.beginGroup("playlistsPlugin");
    QString playlistFormat = se.value("playlistFormat", "m3u").toString();
    int pathType = se.value("pathType", 1).toInt(); // 0 - absolute, 1 - relative
    int playlistType = se.value("playlistType", 0).toInt(); // 0 - simple, 1 - extended
    dir = se.value("dir").toString();
    se.endGroup();

    formatComboBox = new QComboBox(this);
    formatComboBox->addItems(QStringList()<<tr("m3u")
                             <<tr("m3u8")
                             <<tr("pls")
                             <<tr("xspf"));
    formatComboBox->setCurrentIndex(formatComboBox->findText(playlistFormat));

    pathsComboBox = new QComboBox(this);
    pathsComboBox->addItems(QStringList()<<tr("Absolute")
                             <<tr("Relative"));
    pathsComboBox->setCurrentIndex(pathType);

    typeComboBox = new QComboBox(this);
    typeComboBox->addItems(QStringList()<<tr("Simple")
                             <<tr("Extended"));
    typeComboBox->setCurrentIndex(playlistType);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Save"));

    auto *l = new QGridLayout;
    l->addWidget(new QLabel(tr("Format:"), this),0,0);
    l->addWidget(formatComboBox,0,1);
    l->addWidget(new QLabel(tr("Paths:"), this),1,0);
    l->addWidget(pathsComboBox,1,1);
    l->addWidget(new QLabel(tr("Type:"), this),2,0);
    l->addWidget(typeComboBox,2,1);
    l->addWidget(buttonBox,3,0,1,2);

    this->setLayout(l);
}

Dialog::~Dialog()
{
#ifdef QOOBAR_PORTABLE
    QSettings se(QSL("qoobar.ini"),QSettings::IniFormat);
#else
    QSettings se("qoobar","qoobar");
#endif
    se.beginGroup("playlistsPlugin");
    se.setValue("playlistFormat", formatComboBox->currentText());
    se.setValue("pathType", pathsComboBox->currentIndex());
    se.setValue("playlistType", typeComboBox->currentIndex());
    se.setValue("dir", dir);
    se.endGroup();
}

QList<Tag> Dialog::getNewTags()
{
    return oldTags;
}

void Dialog::accept()
{
    if (createPlaylist(formatComboBox->currentText(), pathsComboBox->currentIndex(),
                   typeComboBox->currentIndex()))
        QDialog::accept();
    else
        QDialog::reject();
}

bool Dialog::createPlaylist(const QString &format, int pathType, int type)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save playlist"),
                                                    dir, tr("Playlists (*.%1)").arg(format));
    if (fileName.isEmpty()) return false;
    if (!fileName.endsWith("."+format))
        fileName.append("."+format);

    AbstractPlaylist *playlist = nullptr;
    if (format=="m3u" || format=="m3u8") {
        if (type==0) playlist = new M3UPlaylist(oldTags, format=="m3u8");
        else playlist = new ExtendedM3UPlaylist(oldTags, format=="m3u8");
    }
    else if (format=="pls") {
        if (type==0) playlist = new PlsPlaylist(oldTags);
        else playlist = new ExtendedPlsPlaylist(oldTags);
    }
    else if (format=="xspf") playlist = new XspfPlaylist(oldTags, type!=0);

    if (playlist) playlist->createPlaylist(fileName, pathType);

    dir = QFileInfo(fileName).canonicalPath();

    return true;
}
