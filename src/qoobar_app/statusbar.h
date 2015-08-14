#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
class QLabel;
class Tag;


class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = 0);
public Q_SLOTS:
    void update(const Tag &tag);
    void updateTotalLength(int totalLength);
private:
    QLabel *filenameLabel;
    QLabel *typeLabel;
    QLabel *lengthLabel;
    QLabel *totalLengthLabel;
};

#endif // STATUSBAR_H
