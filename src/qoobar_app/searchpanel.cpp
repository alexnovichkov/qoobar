#include "searchpanel.h"

#include <QtWidgets>

#include "enums.h"
#include "clearlineedit.h"
#include "fancylineedit.h"
#include "qoobarglobals.h"
#include "qocoa/qbutton.h"
#include "application.h"

SearchPanel::SearchPanel(QWidget *parent) :
    QWidget(parent)
{DD
    searchWhatEdit = new SearchLineEdit(this);

    replaceByEdit = new ClearLineEdit(this);
    replaceByEdit->setMinimumWidth(::dpiAwareSize(200,this));

#ifdef Q_OS_MAC
    QButton *closeSearchPanel = new QButton(this, QButton::RoundRect);
    closeSearchPanel->setText(tr("Close"));

    QButton *findNextButton = new QButton(this, QButton::RoundRect);
    QButton *findPrevButton = new QButton(this, QButton::RoundRect);
    QButton *replaceAllButton = new QButton(this, QButton::RoundRect);
    QButton *replaceButton = new QButton(this, QButton::RoundRect);
    QButton *replaceAndFindButton = new QButton(this, QButton::RoundRect);
#else
    QToolButton *closeSearchPanel = new QToolButton(this);
    closeSearchPanel->setIcon(style()->standardPixmap(QStyle::SP_DockWidgetCloseButton));
    closeSearchPanel->setAutoRaise(true);

    QToolButton *findNextButton = new QToolButton(this);
    QToolButton *findPrevButton = new QToolButton(this);
    findNextButton->setText(tr("Find next"));
    findPrevButton->setText(tr("Find previous"));
    findNextButton->setAutoRaise(true);
    findNextButton->setShortcut(QKeySequence::FindNext);
    findNextButton->setToolTip(QString("%1 <font color=gray size=-1>%2</font>")
                               .arg(findNextButton->text())
                               .arg(findNextButton->shortcut().toString()));

    findPrevButton->setAutoRaise(true);
    findPrevButton->setShortcut(QKeySequence::FindPrevious);
    findPrevButton->setToolTip(QString("%1 <font color=gray size=-1>%2</font>")
                               .arg(findPrevButton->text())
                               .arg(findPrevButton->shortcut().toString()));

    QToolButton *replaceAllButton = new QToolButton(this);
    replaceAllButton->setAutoRaise(true);

    QToolButton *replaceButton = new QToolButton(this);
    replaceButton->setAutoRaise(true);

    QToolButton *replaceAndFindButton = new QToolButton(this);
    replaceAndFindButton->setAutoRaise(true);
#endif

    findNextButton->setText(tr("Find next"));
    findPrevButton->setText(tr("Find previous"));
    replaceAllButton->setText(tr("Replace all"));
    replaceButton->setText(tr("Replace"));
    replaceAndFindButton->setText(tr("Replace && Find"));

    connect(closeSearchPanel,SIGNAL(clicked()),this,SLOT(hide()));
    connect(findNextButton,SIGNAL(clicked()),SLOT(findNext()));
    connect(findPrevButton,SIGNAL(clicked()),SLOT(findPrev()));
    connect(replaceAllButton,SIGNAL(clicked()),SIGNAL(replaceAll()));
    connect(replaceButton,SIGNAL(clicked()),SIGNAL(replace()));
    connect(replaceAndFindButton,SIGNAL(clicked()),SIGNAL(replaceAndFind()));


    QGridLayout *searchPanelLayout = new QGridLayout;
    searchPanelLayout->setContentsMargins(0,0,0,0);
    searchPanelLayout->addWidget(new QLabel(tr("Find:"),this),0,0);
    searchPanelLayout->addWidget(searchWhatEdit,0,1);
    searchPanelLayout->addWidget(new QLabel(tr("Replace with:"),this),1,0);
    searchPanelLayout->addWidget(replaceByEdit,1,1);
    searchPanelLayout->addWidget(findPrevButton,0,2);
    searchPanelLayout->addWidget(replaceButton,1,2);
    searchPanelLayout->addWidget(findNextButton,0,3);
    searchPanelLayout->addWidget(replaceAndFindButton,1,3);
    searchPanelLayout->addWidget(replaceAllButton,1,4);
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(0,0,0,0);
    searchLayout->addLayout(searchPanelLayout);
    searchLayout->addWidget(closeSearchPanel,1,Qt::AlignTop | Qt::AlignRight);
    setLayout(searchLayout);
}

bool SearchPanel::caseSensitive() const
{DD
    return searchWhatEdit->caseSensitive();
}

bool SearchPanel::wholeWord() const
{DD
    return searchWhatEdit->wholeWord();
}

bool SearchPanel::useRegularExpressions() const
{DD
    return searchWhatEdit->useRegularExpressions();
}

void SearchPanel::startSearch(const QString &searchWhat)
{DD
    if (!searchWhat.isEmpty())
        searchWhatEdit->setText(searchWhat);
    else
        searchWhatEdit->setFocus();
    searchWhatEdit->selectAll();
}

void SearchPanel::findNext()
{DD
    Q_EMIT find(true);
}

void SearchPanel::findPrev()
{DD
    Q_EMIT find(false);
}

const QString SearchPanel::replaceBy()
{DD
    return replaceByEdit->text();
}

const QString SearchPanel::searchWhat()
{DD
    return searchWhatEdit->text();
}


