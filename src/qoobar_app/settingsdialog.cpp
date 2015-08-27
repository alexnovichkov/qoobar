/**
 * \file settingsdialog.cpp
 * Settings dialog.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 8 Dec 2009
 *
 * Copyright (C) 2009  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Qoobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settingsdialog.h"
#include "configpages.h"
#ifdef HAVE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "application.h"

#ifdef Q_OS_MAC
#include <QtMacExtras>
#include "mactoolbar.h"
#endif

/** These classes were taken from QtCreator */
class Category {
public:
    QString displayName;
    QString icon;
};

class CategoryModel : public QAbstractListModel
{
public:
    CategoryModel(QObject *parent = 0);
    ~CategoryModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void setPages(const QList<ConfigPage*> &pages);
private:
    QList<Category*> m_categories;
    QIcon m_emptyIcon;
};

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{

    QPixmap empty(32, 32);
    empty.fill(Qt::transparent);
    m_emptyIcon = QIcon(empty);
}

CategoryModel::~CategoryModel()
{

    qDeleteAll(m_categories);
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{

    return parent.isValid() ? 0 : m_categories.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{

    switch (role) {
    case Qt::DisplayRole:
        return m_categories.at(index.row())->displayName;
    case Qt::DecorationRole: {
            QIcon icon = QIcon(m_categories.at(index.row())->icon);
            if (icon.isNull())
                icon = m_emptyIcon;
            return icon;
        }
    }
    return QVariant();
}

void CategoryModel::setPages(const QList<ConfigPage*> &pages)
{

    // Clear any previous categories
    qDeleteAll(m_categories);
    m_categories.clear();

    // Put the pages in categories
    Q_FOREACH (ConfigPage *page, pages) {
        Category *category = new Category;
        category->displayName = page->description();
        category->icon = page->iconFilename();
        m_categories.append(category);
    }
    this->beginResetModel();
    endResetModel();
}

class CategoryListViewDelegate : public QStyledItemDelegate
{
public:
    CategoryListViewDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 32));
        return size;
    }
};

class CategoryListView : public QListView
{
public:
    CategoryListView(QWidget *parent = 0) : QListView(parent)
    {

        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        setItemDelegate(new CategoryListViewDelegate(this));
        setSpacing(2);
    }

    virtual QSize sizeHint() const
    {

        int width = sizeHintForColumn(0) + frameWidth() * 2 + 5;
        if (verticalScrollBar()->isVisible())
            width += verticalScrollBar()->width();
        return QSize(width, 100);
    }
};
/** end of taken from QtCreator*/


void SettingsDialog::changePage(const QModelIndex &current)
{
    if (current.isValid())
        pagesWidget->setCurrentIndex(current.row());
}

SettingsDialog::SettingsDialog(QWidget *parent) : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Qoobar settings"));
    this->setWindowModality(Qt::ApplicationModal);

    InterfacePage *page = new InterfacePage;
    connect(page,SIGNAL(retranslate()),this,SLOT(retranslateUI()));
    connect(page,SIGNAL(retranslate()),this,SIGNAL(retranslate()));
//    connect(page,SIGNAL(tableUpdateModeChanged(bool)),this,SIGNAL(tableUpdateModeChanged(bool)));
    configPages << page;
    configPages << new CompletionPage;
    configPages << new WritingPage;
    configPages << new PatternsPage;
    configPages << new UtilitiesPage;
    configPages << new NetworkPage;
    configPages << new PluginsPage;

    pagesWidget = new QStackedWidget;

    Q_FOREACH (ConfigPage *page, configPages)
        pagesWidget->addWidget(page);

#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
    toolBar = addToolBar("ttolbar");
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(int)),pagesWidget,SLOT(setCurrentIndex(int)));

    QActionGroup *ag = new QActionGroup(this);
    for (int i=0; i<configPages.size(); ++i) {
        ConfigPage *page = configPages.at(i);
        QAction *a = new QAction(QIcon(page->iconFilename()), page->description(),this);
        a->setCheckable(true);
        a->setActionGroup(ag);
        if (i==0) a->setChecked(true);
        mapper->setMapping(a,i);
        connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
        toolBar->addAction(a);
    }



#else
    pagesModel = new CategoryModel(this);
    pagesModel->setPages(configPages);

    contentsWidget = new CategoryListView(this);
    contentsWidget->setIconSize(QSize(32, 32));
    contentsWidget->setModel(pagesModel);
    contentsWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    contentsWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(contentsWidget->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(changePage(QModelIndex)));
#endif

    resetSettingsButton = new QPushButton(tr("Reset Settings"),this);
    connect(resetSettingsButton,SIGNAL(clicked()),this,SLOT(resetSettings()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));



    //laying out
    QHBoxLayout *horizontalLayout = new QHBoxLayout;
#ifndef Q_OS_MAC
    horizontalLayout->addWidget(contentsWidget);
#endif
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(resetSettingsButton);
    bottomLayout->addWidget(buttonBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    //mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(bottomLayout);
    QWidget *w = new QWidget(this);
    w->setLayout(mainLayout);
    setCentralWidget(w);

    resize(640,420);
    retranslateUI();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::retranslateUI()
{
    setWindowTitle(tr("Qoobar settings"));
    Q_FOREACH (ConfigPage *page,configPages) page->retranslateUI();
#ifdef Q_OS_MAC
    QList<QAction *> actions = toolBar->actions();
    for (int i=0; i<configPages.size(); ++i) {
        actions[i]->setText(configPages.at(i)->description());
    }
#else
    pagesModel->setPages(configPages);
#endif
    resetSettingsButton->setText(tr("Reset Settings"));
}

void SettingsDialog::accept()
{
    for (int i=0; i<pagesWidget->count(); ++i) {
        ConfigPage *page = qobject_cast<ConfigPage *>(pagesWidget->widget(i));
        if (page)
            page->saveSettings();
    }
    close();
}

void SettingsDialog::resetSettings()
{
    App->resetSettings();
    for (int i=0; i<pagesWidget->count(); ++i) {
        ConfigPage *page = qobject_cast<ConfigPage *>(pagesWidget->widget(i));
        if (page)
            page->setSettings();
    }
}
