#include "treeview.h"
#include "qoobarglobals.h"
#include "application.h"
#include "columnsdialog.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "tab.h"
#include "tagsreaderwriter.h"
#include "enums.h"
#include "mainwindow.h"
#include "model.h"

#ifdef Q_OS_MAC
class ColumnSelectorButton : public QToolButton
{
public:
    explicit ColumnSelectorButton(QHeaderView *header, QWidget*parent=0) : QToolButton(parent),
        m_header(header)
    {}
    void paintEvent(QPaintEvent * pe)
    {
        int w = width();
        int h = m_header->height();
        w++;

        QPainter p(this);
        p.setClipRect(pe->rect());

        QStyleOptionHeader opt;
        opt.initFrom(m_header);
        opt.state |= QStyle::State_Horizontal |
                     QStyle::State_Enabled |
                     QStyle::State_Raised;
        opt.rect = QRect(0,0, w,h);
        style()->drawControl(QStyle::CE_Header, &opt, &p, m_header);
        p.end();
    }
private:
    QHeaderView *m_header;
};
#endif

TreeView::TreeView(Tab *parent) : QTreeView(parent)
{DD;
    tab=parent;

    setMouseTracking(true);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);

    showAct = new QAction(tr("Show/hide columns..."), this);
    connect(showAct,SIGNAL(triggered()),this,SLOT(adjustDisplayedTags()));

#ifdef Q_OS_MAC
    ColumnSelectorButton *selector = new ColumnSelectorButton(this->header(), this);
    this->addScrollBarWidget(selector,Qt::AlignTop);
    selector->setDefaultAction(showAct);

    setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
#endif

    if (!App->columns167.isEmpty()) {
        header()->restoreState(App->columns167);
    }
    connect(App,SIGNAL(settingsCleared()),SLOT(resetHeader()));

    retranslateUi();

    mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(int)), this, SIGNAL(moveToTab(int)));
}

void TreeView::resetHeader()
{DD;
    header()->setSortIndicatorShown(true);
#ifdef Q_OS_LINUX
    header()->setMinimumSectionSize(fontMetrics().width(QSL("5555")));
#endif
    header()->SETSECTIONRESIZEMODE(COL_TRACKNUMBER, QHeaderView::ResizeToContents); //No.
    header()->SETSECTIONRESIZEMODE(COL_LENGTH, QHeaderView::ResizeToContents); //length
    header()->SETSECTIONRESIZEMODE(COL_SAVEICON, QHeaderView::ResizeToContents); //saved
    header()->SETSECTIONRESIZEMODE(COL_REPLAYGAIN, QHeaderView::ResizeToContents); //saved
    header()->SETSECTIONRESIZEMODE(COL_IMAGE, QHeaderView::ResizeToContents); //saved
    header()->setStretchLastSection(false);
    header()->setContextMenuPolicy(Qt::ActionsContextMenu);
    header()->SETSECTIONSCLICKABLE(true);

    connect(header(),SIGNAL(geometriesChanged()),SLOT(updateTreeHeaderGeometry()));
    connect(header(),SIGNAL(sectionMoved(int,int,int)),SLOT(updateTreeHeaderGeometry()));
    connect(header(),SIGNAL(sectionResized(int,int,int)),SLOT(updateTreeHeaderGeometry()));
    connect(header(),SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),SLOT(sortIndicatorChanged(int,Qt::SortOrder)));

    header()->addAction(showAct);
}

void TreeView::updateTreeHeaderGeometry()
{DD;
    App->columns167 = header()->saveState();
}

void TreeView::retranslateUi()
{DD;
    showAct->setText(tr("Show/hide columns..."));
}

void TreeView::scrollToRow(int row)
{DD;
    if (row<=0) scrollToTop();
    else if (row>=model()->rowCount()) scrollToBottom();
    else scrollTo(model()->index(row,0));
}

void TreeView::adjustDisplayedTags()
{DD;
    ColumnsDialog dialog(this);
    dialog.exec();
}

void TreeView::sortIndicatorChanged(int section, Qt::SortOrder order)
{DD;
    int sortType = SortString;
    if (section==COL_LENGTH) sortType = SortTime;
    Q_EMIT sortRequested(section, order, sortType);
}

void TreeView::contextMenuEvent(QContextMenuEvent *event)
{DD;
    QMenu *menu = new QMenu(this);
    menu->insertActions(0,this->actions());

    if (tab) {
        QAction *moveToTabAct = new QAction(tr("Move to the tab..."),menu);
        moveToTabAct->setEnabled(this->selectionModel()->hasSelection());
        QMenu *moveToTabMenu = new QMenu(this);
        QMap<int, QString> tabNames = tab->allTabsNames();
        tabNames.insert(-1,tr("New tab"));
        Q_FOREACH (int tabID, tabNames.keys()) {
            QAction *a = new QAction(tabNames.value(tabID),moveToTabMenu);
            moveToTabMenu->insertAction(0,a);
            connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
            mapper->setMapping(a, tabID);
        }
        moveToTabAct->setMenu(moveToTabMenu);
        menu->insertAction(0,moveToTabAct);

        if (tab->allSelectedFilesAreMP3()) {
            QAction *a = new QAction(tr("MP3 tag types..."),menu);
            connect(a, SIGNAL(triggered()), this, SIGNAL(showMP3TagsDialog()));
            menu->insertAction(0,a);
        }
    }

    menu->exec(event->globalPos());
    menu->deleteLater();
}
