/**
 * \file tagseditor.cpp
 * Editing tags dialog.
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

#include <QtWidgets>

#include "tagseditor.h"
#include "application.h"
#include "completerdelegate.h"
#include "lineedit.h"
//#include "qoobarglobals.h"
#include "logging.h"
#include "tagsrenderer.h"
#include "legendbutton.h"
#include "texteditdelegate.h"
#include "searchpanel.h"
#include "sections.h"
#include "sectiondelegatehighlighter.h"
#include "model.h"
#include "enums.h"
#include "stringroutines.h"
#include "qbutton.h"
#include "imagedialog.h"
#include "styledbar.h"
#include "qoobarhelp.h"

#define USE_CONCURRENT
#ifdef USE_CONCURRENT
#include <QtConcurrent/QtConcurrent>
#endif



TagsEditDialog::TagsEditDialog(int type, const QString &caption,
                       const QStringList &list,
                       const QString &text, bool p,
                       QWidget *parent) : QDialog(parent)
{DD
    setWindowTitle(caption);
    setWindowModality(Qt::WindowModal);

    m_type=type;
    plain=p;
    edit=0;
    pedit=0;
    highlighter=0;

    count=list.count();
    QLabel *mLabel = new QLabel(tr("Multiple values:"));

    legendButton = new LegendButton(this);
    legendButton->setCategories(LegendButton::WritablePlaceholders | LegendButton::ReadOnlyPlaceholders);
    connect(legendButton,SIGNAL(placeholderChosen(QString)),SLOT(insertLegend(QString)));
    legendButton->setFocusPolicy(Qt::NoFocus);
    legendButton->retranslateUi();

    table = new QTableWidget(count,1);
    table->horizontalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    int height=table->verticalHeader()->sectionSize(0);
    QHeaderView::ResizeMode rm = QHeaderView::Interactive;
    if (plain) rm=QHeaderView::Fixed;
    table->verticalHeader()->setSectionResizeMode(rm);
    table->verticalHeader()->setDefaultSectionSize(height);
    for (int i = 0; i < count; ++i)
        table->setItem(i, 0, new QTableWidgetItem(list.at(i)));
    connect(table,SIGNAL(cellClicked(int,int)),this,SLOT(cellClicked(int,int)));
    connect(table,SIGNAL(currentCellChanged(int,int,int,int)),this,SLOT(currentCellChanged(int)));

    hideIcon = QIcon::fromTheme("fold");
    showIcon = QIcon::fromTheme("unfold");

#ifndef Q_OS_MAC
    double h = QFontMetricsF(App->charsFont).height();

    scroll = new QScrollArea;
    //scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll->setSizePolicy(QSizePolicy::Preferred, scroll->sizePolicy().verticalPolicy());
    int sbw = 10;
    int factor = 4;
    if (devicePixelRatio()==1) factor = 5;
//    scroll->setMinimumWidth(::dpiAwareSize(h*factor+sbw,this));
//    scroll->setMaximumWidth(::dpiAwareSize(h*factor+sbw,this));
    //scroll->setMinimumWidth(h*factor+sbw);
    //scroll->setMaximumWidth(h*factor+sbw);
    scroll->setFocusPolicy(Qt::NoFocus);
    QWidget *charsWidget = new QWidget(this);
    charsWidget->setFocusPolicy(Qt::NoFocus);
//    charsWidget->setMinimumWidth(::dpiAwareSize(h*4,this));
//    charsWidget->setMaximumWidth(::dpiAwareSize(h*4,this));
//    charsWidget->setMinimumWidth(h*4);
//    charsWidget->setMaximumWidth(h*4);

    QGridLayout *charsLayout = new QGridLayout(this);
    charsLayout->setContentsMargins(0,0,0,0);
    charsLayout->setSpacing(0);
    int row=0; int col=0;
    QString chars = App->chars;

    Q_FOREACH (const QChar &c, chars) {
        QAction *a=new QAction(QString(c),this);
        a->setToolTip(tr("%1 (%2 with Shift key)").arg(c).arg(c.toUpper()));
        a->setFont(App->charsFont);
        connect(a,SIGNAL(triggered()),SLOT(insertFromCharsList()));
        QToolButton *b = new QToolButton(this);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
//        b->setFixedSize(h+::dpiAwareSize(4,this),h+::dpiAwareSize(4,this));
        //b->setFixedSize(::dpiAwareSize(h,this),dpiAwareSize(h,this));
        b->setDefaultAction(a);
        if (col==4) {
            col=0; row++;
        }
        charsLayout->addWidget(b,row,col++);
    }

    charsWidget->setLayout(charsLayout);
    scroll->setWidget(charsWidget);

    toggleCharsAct = new QAction(this);
    toggleCharsAct->setShortcut(tr("Ctrl+Meta+Space"));
    connect(toggleCharsAct,SIGNAL(triggered()),this,SLOT(toggleCharsWidget()));

    QToolButton *toggleCharsButton = new QToolButton(this);
    toggleCharsButton->setAutoRaise(true);
    toggleCharsButton->setDefaultAction(toggleCharsAct);

    if (App->charsShown) {
        toggleCharsAct->setIcon(hideIcon);
        toggleCharsAct->setToolTip(tr("Hide characters"));
    }
    else {
        scroll->hide();
        toggleCharsAct->setIcon(showIcon);
        toggleCharsAct->setToolTip(tr("Show characters"));
    }
#endif

    if (plain) {
        edit = new LineEdit(false,this);
        if (!text.startsWith(QLS("<<"))) edit->setText(list.at(0));
        edit->setCursorPosition(0);
        connect(edit,SIGNAL(tagChanged(int,bool)),this,SLOT(handleSentTag(int,bool)));
        connect(table,SIGNAL(cellChanged(int,int)),edit,SLOT(clear()));
        edit->setCompleter(new MyCompleter(m_type,edit));

        CompleterDelegate *delegate = new CompleterDelegate(m_type,true,this);
        highlighter = new SectionDelegateHighlighter();
        delegate->setHighlighter(highlighter);
        connect(delegate,SIGNAL(tagChanged(int,bool)),this,SLOT(handleSentTag(int,bool)));
        connect(delegate,SIGNAL(selectionChanged()),this,SLOT(tableCellSelectionChanged()));
        connect(delegate,SIGNAL(closeEditor(QWidget*)),this,SLOT(delegateDidFinishEditing()));
        table->setItemDelegate(delegate);
    }
    else {
        pedit = new QPlainTextEdit(this);
        pedit->setMinimumHeight(80);
        pedit->setMaximumHeight(200);
        if (!text.startsWith(QLS("<<")))  pedit->setPlainText(list.at(0));

        table->setItemDelegate(new TextEditDelegate(this));
        connect(table,SIGNAL(cellChanged(int,int)),pedit,SLOT(clear()));
    }


    QList<QAction *> acts;
    QSignalMapper *mapper = new QSignalMapper(this);
    QMenu *collectIntoMenu = new QMenu(this);
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i) {
        QAction *a=new QAction(App->currentScheme->localizedFieldName[i],this);
        mapper->setMapping(a,i);
        connect(a,SIGNAL(triggered()),mapper,SLOT(map()));
        acts << a;
    }
    collectIntoMenu->addActions(acts);
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(collectTags(int)));

    QAction *collect = new QAction(tr("Collect for future use"),this);
    connect(collect,SIGNAL(triggered()),this,SLOT(collectTags()));
    collect->setMenu(collectIntoMenu);

    searchPanel = new SearchPanel(this);
    connect(searchPanel,SIGNAL(find(bool)),SLOT(find(bool)));
    connect(searchPanel,SIGNAL(replace()),SLOT(replace()));
    connect(searchPanel,SIGNAL(replaceAll()),SLOT(replaceAll()));
    connect(searchPanel,SIGNAL(replaceAndFind()),SLOT(replaceAndFind()));

    QAction *startSearchAct = new QAction(QIcon::fromTheme("edit-find"),tr("Find/Replace"),this);
#ifndef Q_OS_MAC
    startSearchAct->setShortcut(QKeySequence::Find);
#endif
    connect(startSearchAct,SIGNAL(triggered()),SLOT(startSearch()));
    addAction(startSearchAct);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#ifdef Q_OS_MAC
    QButton *helpButton = new QButton(this,QButton::HelpButton);
#else
    QPushButton *helpButton = buttonBox->addButton(QDialogButtonBox::Help);
#endif
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));


    QSignalMapper *operationsMapper = new QSignalMapper(this);
    connect(operationsMapper,SIGNAL(mapped(QString)),SLOT(handleOperation(QString)));

    static const struct Operation {
        const char *text;
        const char *icon;
        const char *map;
    } operations[] = {
        {QT_TR_NOOP("lower"),"small","lower"},
        {QT_TR_NOOP("UPPER"),"caps","upper"},
        {QT_TR_NOOP("Only first up"),"word","caps3"},
        {QT_TR_NOOP("Every First Up"),"everyword","caps"},
        {QT_TR_NOOP("Simplify whitespaces"),"trim","simplify"},
        {QT_TR_NOOP("Remove diacritics"),"diacritics","ansi"},
        {QT_TR_NOOP("Transliterate"),"transliterate","transliterate"},
        {QT_TR_NOOP("Fix encoding"),"fix","recode"}
    };



#ifdef Q_OS_MAC
    StyledBar *operationsToolBar = new StyledBar(this);
    QHBoxLayout *l = new QHBoxLayout;
    l->setSpacing(0);
    l->setMargin(0);
    for (int i=0; i<8; ++i) {
        QAction *a = new QAction(QIcon::fromTheme(operations[i].icon),
                                 operations[i].text,this);
        connect(a,SIGNAL(triggered()),operationsMapper,SLOT(map()));
        operationsMapper->setMapping(a, operations[i].map);
        QToolButton *tb = new FancyToolButton(this);
        tb->setDefaultAction(a);
        l->addWidget(tb);
    }
    QToolButton *tb=new FancyToolButton(this);
    tb->setDefaultAction(startSearchAct);
    l->addWidget(tb);
    tb=new QToolButton(this);
    tb->setDefaultAction(collect);
    l->addWidget(new StyledSeparator(this));
    l->addWidget(tb);
    l->addStretch();
    operationsToolBar->setLayout(l);
#else
    QToolBar *operationsToolBar = new QToolBar(this);
    operationsToolBar->addWidget(toggleCharsButton);
    operationsToolBar->addSeparator();

    operationsToolBar->setMovable(false);
    operationsToolBar->setIconSize(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));

    for (int i=0; i<8; ++i)
        operationsMapper->setMapping(
                    operationsToolBar->addAction(QIcon::fromTheme(operations[i].icon),
                                                 operations[i].text,
                                                 operationsMapper,
                                                 SLOT(map()))
                    ,operations[i].map);
    operationsToolBar->insertSeparator(operationsToolBar->actions().at(4));
    operationsToolBar->addAction(startSearchAct);
    operationsToolBar->addSeparator();
    operationsToolBar->addAction(collect);
#endif

    //synthesizing
    QGridLayout *tagsEditorLayout = new QGridLayout;
#ifndef Q_OS_MAC
    tagsEditorLayout->addWidget(scroll,1,0,plain?3:4,1);
#endif
    if (plain) tagsEditorLayout->addWidget(edit,1,1);
    else tagsEditorLayout->addWidget(pedit,1,1,2,1);
    tagsEditorLayout->addWidget(legendButton,1,2);
    int pos=2; if (!plain) pos++;
    tagsEditorLayout->addWidget(mLabel,pos,1,1,2); pos++;
    tagsEditorLayout->addWidget(table,pos,1,1,2); pos++;
    tagsEditorLayout->addWidget(searchPanel,pos,0,1,3); pos++;
#ifdef Q_OS_MAC
    QHBoxLayout *boxL = new QHBoxLayout;
    boxL->addWidget(helpButton);
    boxL->addWidget(buttonBox);
    tagsEditorLayout->addLayout(boxL,pos,0,1,3);
#else
    tagsEditorLayout->addWidget(buttonBox,pos,0,1,3);
#endif
    setLayout(tagsEditorLayout);
    this->layout()->setMenuBar(operationsToolBar);

    searchPanel->hide();

    resize(qApp->primaryScreen()->availableSize()/2);
}


TagsEditDialog::~TagsEditDialog()
{DD
    delete highlighter;
    model->setCurrentIndex(-1);
}

void TagsEditDialog::collectTags()
{DD
    collectTags(m_type);
}

void TagsEditDialog::collectTags(int tagID)
{DD
    if (tagID>=App->currentScheme->tagsCount()) return;

    QString collected;
    QLineEdit *e=qobject_cast<QLineEdit *>(qApp->focusWidget());
    if (e && e->hasSelectedText()) collected = e->selectedText();

    QPlainTextEdit *e1=qobject_cast<QPlainTextEdit *>(qApp->focusWidget());
    if (e1 && e1->textCursor().hasSelection()) collected = e1->textCursor().selectedText();

    if (collected.isEmpty()) return;
    App->autocompletions->update(tagID, QStringList(collected));

    if (plain) edit->setCompleter(new MyCompleter(tagID,edit));
}

void TagsEditDialog::handleOperation(const QString &type)
{DD
    QLineEdit *e=qobject_cast<QLineEdit *>(qApp->focusWidget());
    if (e) {
        e->insert(operatedString(e->selectedText(), type));
        return;
    }

    QPlainTextEdit *e1=qobject_cast<QPlainTextEdit *>(qApp->focusWidget());
    if (e1) {
        e1->insertPlainText(operatedString(e1->textCursor().selectedText(), type));
        return;
    }

    QList<QTableWidgetItem*> items = table->selectedItems();
    for (int i=0; i<items.size(); ++i) {
        QString s=items.at(i)->text();
        items.at(i)->setText(operatedString(s,type));
    }
}

void TagsEditDialog::startSearch()
{DD
    if (searchPanel->isHidden()) searchPanel->show();
    QLineEdit *e=qobject_cast<QLineEdit *>(table->cellWidget(table->currentRow(),0));
    QString searchWhat;
    if (e) {
        if (e->hasSelectedText()) searchWhat=e->selectedText();
    }
    searchPanel->startSearch(searchWhat);
}



void TagsEditDialog::find(const bool forward)
{DD
    QString searchWhat = searchPanel->searchWhat();
    if (searchWhat.isEmpty()) return;

    rx = createRegExp(searchPanel->caseSensitive(), searchPanel->useRegularExpressions(),
                            searchPanel->wholeWord(), searchWhat);
    captures.clear();

    int row = table->currentRow();

    QLineEdit *e=qobject_cast<QLineEdit *>(table->cellWidget(row,0));
    if (e) {
        //search in the current line
        int pos = e->cursorPosition();
        if (e->hasSelectedText()) {
            if (forward)
                pos = e->selectionStart()+e->selectedText().length();
            else
                pos = e->selectionStart()-1;
        }
        if (forward)
            pos = rx.indexIn(e->text(),pos);
        else
            pos = rx.lastIndexIn(e->text(),pos);
        if (pos>=0) {
            e->setSelection(pos,rx.matchedLength());
            captures = rx.capturedTexts();
            captures.removeFirst();
            return;
        }
    }

    if (!e) row = forward ? 0 : count-1;
    else row = forward ? row+1 : row-1;
    if (row==count || row == -1) {
        QMessageBox::information(this,tr("Qoobar"),tr("Nothing found"));
        return;
    }

    table->setCurrentItem(table->item(row,0));
    table->editItem(table->item(row,0));
    e = qobject_cast<QLineEdit *>(table->cellWidget(row,0));
    if (!e) return;
    int pos = forward ? rx.indexIn(e->text(),0) : rx.lastIndexIn(e->text(),-1);
    if (pos>=0) {
        e->setSelection(pos,rx.matchedLength());
        captures = rx.capturedTexts();
        captures.removeFirst();
    }
    else find(forward);
}

void TagsEditDialog::replace()
{DD
    int row = table->currentRow();
    QLineEdit *e=qobject_cast<QLineEdit *>(table->cellWidget(row,0));
    if (!e) return;
    if (e->hasSelectedText()) e->insert(searchPanel->replaceBy(captures));
}

void TagsEditDialog::replaceAndFind()
{DD
    int row = table->currentRow();
    QLineEdit *e=qobject_cast<QLineEdit *>(table->cellWidget(row,0));
    if (e)
        if (e->hasSelectedText()) {
            e->insert(searchPanel->replaceBy(captures));
        }
    find(true);
}

void TagsEditDialog::replaceAll()
{DD
    QString searchWhat = searchPanel->searchWhat();
    if (searchWhat.isEmpty()) return;
    rx=createRegExp(searchPanel->caseSensitive(), searchPanel->useRegularExpressions(),
                            searchPanel->wholeWord(), searchWhat);
    captures.clear();

    for (int i=0; i<count; ++i) {
        QString s = table->item(i,0)->text();
        s = s.replace(rx, searchPanel->replaceBy());
        table->item(i,0)->setText(s);
    }
}

void TagsEditDialog::showHelp()
{DD;
    Qoobar::showHelp(QSL("tags_editing"));
}

void TagsEditDialog::tableCellSelectionChanged()
{DD
    int baseIndex = table->currentRow();
    QLineEdit *e=qobject_cast<QLineEdit *>(qApp->focusWidget());
    if (!e) return;
    if (!e->hasSelectedText()) {
        for (int i=0; i<count; ++i) {
            if (i==baseIndex) continue;
            table->item(i,0)->setData(Qt::UserRole+1,QVariant());
        }
        return;
    }

    if (!(qApp->keyboardModifiers() & Qt::ControlModifier)) return;

    QStringList source;
    for (int i=0; i<count; ++i) {
        if (i==baseIndex) source << e->text();
        else source << table->item(i,0)->text();
    }

    const QString baseString=source.at(baseIndex);
    const int start = e->selectionStart();
    const int end = start + e->selectedText().length() - 1;

#ifdef USE_CONCURRENT
    QList<QPoint> destination = QtConcurrent::blockingMapped(source, GetSection(baseString,start,end));
    for (int i=0; i<count; ++i) {
        if (i==baseIndex) continue;
        table->item(i,0)->setData(Qt::UserRole+1,destination.at(i));
    }
#else
    GetSection gs(baseString,start,end);
    for (int i=0; i<count; ++i) {
        if (i==baseIndex) continue;
        table->item(i,0)->setData(Qt::UserRole+1, gs(source.at(i)));
    }
#endif
}

void TagsEditDialog::handleSentTag(int tagID,bool fromTable)
{DD
    QStringList destination;
    QString s;

    if (edit) {
        if (edit->hasSelectedText() && edit->hasFocus()) s = edit->selectedText();
    }
    else if (pedit) {
        if (pedit->textCursor().hasSelection() && pedit->hasFocus())
            s = pedit->textCursor().selectedText();
    }
    if (!s.isEmpty())
        destination = QVector<QString>(count,s).toList();
    else {
        QLineEdit *lineEdit=qobject_cast<QLineEdit *>(qApp->focusWidget());
        if (!lineEdit) return;
        QString selectedText=lineEdit->selectedText();
        if (selectedText.isEmpty()) return;

        if (fromTable) {
            QStringList source;
            for (int i=0; i<count; ++i) source << table->item(i,0)->text();
            const int baseIndex = table->currentRow();
            Q_ASSERT(baseIndex >= 0 && baseIndex < table->rowCount());
            const QString baseString=source.at(baseIndex);
            const int start = lineEdit->selectionStart();
            const int end = start + selectedText.length() - 1;

#ifdef USE_CONCURRENT
            destination = QtConcurrent::blockingMapped(source, GetSectionString(baseString,start,end));
#else
            GetSectionString gs(baseString,start,end);
            for (int i=0; i<count; ++i) {
                destination << gs(source.at(i));
            }
#endif
        }
        else
            destination = QVector<QString>(count,selectedText).toList();
    }
    Q_EMIT tagsSent(tagID, destination);

    if (tagID == m_type) {//reset changed tags
        for (int i=0; i<destination.size(); ++i)
            table->item(i,0)->setText(destination.at(i));
    }
}

void TagsEditDialog::toggleCharsWidget()
{DD;
#ifndef Q_OS_MAC
    if (scroll->isVisible()) {
        scroll->hide();
        toggleCharsAct->setIcon(showIcon);
        toggleCharsAct->setToolTip(tr("Show characters"));
        App->charsShown = false;
    }
    else {
        scroll->show();
        toggleCharsAct->setIcon(hideIcon);
        toggleCharsAct->setToolTip(tr("Hide characters"));
        App->charsShown = true;
    }
#endif
}

void TagsEditDialog::insertLegend(const QString &s)
{DD
    if (plain) edit->insert(s);
    else pedit->insertPlainText(s);
}

void TagsEditDialog::cellClicked(int row, int col)
{DD
    table->editItem(table->item(row,col));
}

void TagsEditDialog::currentCellChanged(int curRow)
{DD;
    model->setCurrentIndex(model->indexAtIndexInSelection(curRow));
    Q_EMIT rowSelected(model->indexAtIndexInSelection(curRow));
}

void TagsEditDialog::insertFromCharsList()
{DD
    QAction*a= qobject_cast<QAction*>(sender());
    if (!a) return;

    QString s=a->text();
    if (qApp->keyboardModifiers() & Qt::ShiftModifier) s=s.toUpper();

    QLineEdit *e=qobject_cast<QLineEdit *>(qApp->focusWidget());
    if (e) e->insert(s);

    QPlainTextEdit *e1 = qobject_cast<QPlainTextEdit *>(qApp->focusWidget());
    if (e1) e1->insertPlainText(s);
}


void TagsEditDialog::spread(const QString &pattern)
{DD
    if (pattern.isEmpty()) {
        for (int i=0; i<count; ++i)
            newValues << table->item(i,0)->text();
    }
    else
        newValues = model->tagsByPattern(m_type, pattern);
}

QStringList TagsEditDialog::getList()
{DD
    return newValues;
}

void TagsEditDialog::setModel(Model *model)
{DD
    this->model = model;
}

void TagsEditDialog::accept()
{DD;
    if (edit) edit->setFocus();
    if (pedit) pedit->setFocus();

    if (plain)
        spread(edit->text());
    else
        spread(pedit->toPlainText());

    if (plain && edit->completer())
        edit->completer()->popup()->hide();


    QDialog::accept();
}

void TagsEditDialog::delegateDidFinishEditing()
{DD
    for (int i=0; i<count; ++i)
        table->item(i,0)->setData(Qt::UserRole+1,QVariant());
}

