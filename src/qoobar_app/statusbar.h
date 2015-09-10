#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
class QLabel;
class Tag;
//class QAction;

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = 0);
//    void updateIcon();
    void retranslateUI();
public Q_SLOTS:
    void update(const Tag &tag);
    void updateTotalLength(int totalLength);
//private Q_SLOTS:
//    void switchFilesProperties();
private:
    QLabel *filenameLabel;
    QLabel *typeLabel;
    QLabel *lengthLabel;
    QLabel *totalLengthLabel;
//    QAction *unfoldAct;
};

#endif // STATUSBAR_H
