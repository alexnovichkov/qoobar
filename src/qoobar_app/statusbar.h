#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include "tagger.h"
#include <QLabel>

class  ElidingLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode DESIGNABLE true)

public:
    explicit ElidingLabel(QWidget *parent = 0);
    explicit ElidingLabel(const QString &text, QWidget *parent = 0);

    Qt::TextElideMode elideMode() const;
    void setElideMode(const Qt::TextElideMode &elideMode);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Qt::TextElideMode m_elideMode;
};

class PropertiesPanel: public QWidget
{
    Q_OBJECT
public:
    explicit PropertiesPanel(QWidget *parent=0);
    void retranslateUI();
    void updateLengthContents();
    void updateFileName();
    void updateSizeContents();
    void updateHover(const Tag& tag);
    void updateSelected();

private:
    QLabel *fileIconLabel;
    ElidingLabel *fileNameLabel;
    QLabel *readOnlyLabel;
    QLabel *typeLabel;
    QLabel *sizeLabel;
    QLabel *sizeContentsLabel;
    QLabel *lengthLabel;
    QLabel *lengthContentsLabel;
    QLabel *selectedLabel;
    QLabel *selectedLengthLabel;

    Tag currentHover;
public:
    int overallCount;
    int selectedCount;
    int overallLength;
    int selectedLength;
};

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = 0);
    void retranslateUI();
public Q_SLOTS:
    void update(const Tag &tag);
    void updateTotalLength(int, int);
    void updateSelectedLength(int, int);
private:
    PropertiesPanel *panel;
};

#endif // STATUSBAR_H
