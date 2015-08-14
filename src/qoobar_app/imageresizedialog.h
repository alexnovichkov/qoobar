#ifndef IMAGERESIZEDIALOG_H
#define IMAGERESIZEDIALOG_H

#include <QDialog>

#include "coverimage.h"

class QSpinBox;
class QButtonGroup;
class QCheckBox;

class ImageResizeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageResizeDialog(CoverImage &img, QWidget *parent = 0);
    int newWidth;
    int newHeight;
Q_SIGNALS:

public Q_SLOTS:
    void accept();
private Q_SLOTS:
    void widthChanged(int width);
    void heightChanged(int height);
private:
    CoverImage image;
    QSpinBox *widthBox;
    QSpinBox *heightBox;
    QCheckBox *keepCheckBox;
    int origWidth;
    int origHeight;
    float proportion;
};

#endif // IMAGERESIZEDIALOG_H
