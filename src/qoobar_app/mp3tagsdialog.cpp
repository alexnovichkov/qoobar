#include "mp3tagsdialog.h"
#include <QtWidgets>
#include "application.h"
#include "enums.h"
#include "qoobarglobals.h"

MP3TagsDialog::MP3TagsDialog(int fileCount, QWidget *parent) :
    QDialog(parent), m_tagTypes(0)
{DD;

    setWindowTitle(tr("MP3 tag types (%n file(s))",0,fileCount));

    id3v1CheckBox = new QCheckBox(QSL("ID3v1"),this);
    id3v2CheckBox = new QCheckBox(QSL("ID3v2"),this);
    apeCheckBox = new QCheckBox(QSL("APE"),this);

    id3v1CheckBox->setChecked(App->id3v1Synchro==ID3V1_UPDATE_ALWAYS);
    id3v2CheckBox->setChecked(App->mp3writeid3);
    apeCheckBox->setChecked(App->mp3writeape);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(tr("Update files"), QDialogButtonBox::AcceptRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(id3v1CheckBox);
    l->addWidget(id3v2CheckBox);
    l->addWidget(apeCheckBox);
    l->addWidget(buttonBox);
    setLayout(l);
    //TODO: this->devicePixelRatioF()
    resize(::dpiAwareSize({200,100},this));
}

void MP3TagsDialog::accept()
{DD;
    if (id3v1CheckBox->isChecked())
        m_tagTypes |= ID3V1;
    if (id3v2CheckBox->isChecked())
        m_tagTypes |= ID3V2;
    if (apeCheckBox->isChecked())
        m_tagTypes |= APE;
    QDialog::accept();
}

