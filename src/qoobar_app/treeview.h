#ifndef TREEVIEW_H
#define TREEVIEW_H

class Tag;
class Settings;
class Tab;
class QSignalMapper;
#include "tagger.h"
#include <QTreeView>

class TreeView : public QTreeView
{
    Q_OBJECT
public:
    TreeView(Tab *parent = 0);
public Q_SLOTS:
    void resetHeader();
    void retranslateUi();
    void scrollToRow(int row);
private Q_SLOTS:
    void updateTreeHeaderGeometry();
    void adjustDisplayedTags();
    void sortIndicatorChanged(int section, Qt::SortOrder order);
Q_SIGNALS:
    void moveToTab(int);
    void showMP3TagsDialog();
    void sortRequested(int section, Qt::SortOrder order, int sortType);
protected:
    void contextMenuEvent(QContextMenuEvent *);

private:
    int currentLine;
    QSignalMapper *mapper;
    Tab *tab;
    QAction *showAct;
};

#endif // TREEVIEW_H
