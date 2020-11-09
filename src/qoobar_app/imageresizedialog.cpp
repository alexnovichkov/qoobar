#include "imageresizedialog.h"

#include <QtWidgets>
#include "qoobarglobals.h"

ImageResizeDialog::ImageResizeDialog(CoverImage &img, QWidget *parent) :
    QDialog(parent), image(img)
{DD;
    setWindowTitle(tr("Resize cover art"));

    QPixmap x;
    x.loadFromData(image.pixmap());
//    x.setDevicePixelRatio(devicePixelRatioF());
    origWidth = x.width();
    origHeight = x.height();
    newWidth = origWidth;
    newHeight = origHeight;
    proportion = (float)origWidth / (float)origHeight;

    widthBox = new QSpinBox(this);
    heightBox = new QSpinBox(this);
    widthBox->setSuffix("px");
    heightBox->setSuffix("px");
    widthBox->setRange(1,origWidth*100);
    heightBox->setRange(1,origHeight*100);
    widthBox->setValue(origWidth);
    heightBox->setValue(origHeight);
    connect(widthBox, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
    connect(heightBox, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));

    keepCheckBox = new QCheckBox(tr("Keep proportions"),this);
    keepCheckBox->setChecked(true);


    QGridLayout *l = new QGridLayout;
    l->addWidget(new QLabel(tr("Width"),this),0,0);
    l->addWidget(widthBox,0,1);
    l->addWidget(new QLabel(tr("Height"),this),1,0);
    l->addWidget(heightBox,1,1);
    l->addWidget(keepCheckBox,2,0,1,2);
    l->addWidget(new QLabel(tr("Original width: %1 px").arg(origWidth),this),3,0,1,2);
    l->addWidget(new QLabel(tr("Original height: %1 px").arg(origHeight),this),4,0,1,2);


    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    l->addWidget(buttonBox,5,0,1,2);
    setLayout(l);
}

void ImageResizeDialog::accept()
{DD;
    newWidth = widthBox->value();
    newHeight = heightBox->value();
    QDialog::accept();
}

void ImageResizeDialog::widthChanged(int width)
{DD;
    if (keepCheckBox->isChecked()) {
        heightBox->blockSignals(true);
        heightBox->setValue( int(width / proportion));
        heightBox->blockSignals(false);
    }
}

void ImageResizeDialog::heightChanged(int height)
{DD;
    if (keepCheckBox->isChecked()) {
        widthBox->blockSignals(true);
        widthBox->setValue( int(height * proportion));
        widthBox->blockSignals(false);
    }
}
