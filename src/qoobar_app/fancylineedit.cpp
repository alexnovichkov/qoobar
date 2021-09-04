/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "fancylineedit.h"
#include <QEvent>

#include <QString>
#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QMenu>
#include <QMouseEvent>
#include <QLabel>
#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QPaintEvent>
#include <QtDebug>

#include "enums.h"
#include "qoobarglobals.h"
#include "application.h"

/*! Opens a menu at the specified widget position.
 * This functions computes the position where to show the menu, and opens it with
 * QMenu::exec().
 * \param menu The menu to open
 * \param widget The widget next to which to open the menu
 */
static void execMenuAtWidget(QMenu *menu, QWidget *widget)
{DD;
    QPoint p;
    QRect screen = App->primaryScreen()->availableGeometry();
    QSize sh = menu->sizeHint();
    QRect rect = widget->rect();
    if (widget->isRightToLeft()) {
        if (widget->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
            p = widget->mapToGlobal(rect.bottomRight());
        } else {
            p = widget->mapToGlobal(rect.topRight() - QPoint(0, sh.height()));
        }
        p.rx() -= sh.width();
    } else {
        if (widget->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
            p = widget->mapToGlobal(rect.bottomLeft());
        } else {
            p = widget->mapToGlobal(rect.topLeft() - QPoint(0, sh.height()));
        }
    }
    p.rx() = qMax(screen.left(), qMin(p.x(), screen.right() - sh.width()));
    p.ry() += 1;

    menu->exec(p);
}

/*!
    \class Utils::FancyLineEdit

    \brief A line edit with an embedded pixmap on one side that is connected to
    a menu.

    Additionally, it can display a grayed hintText (like "Type Here to")
    when not focused and empty. When connecting to the changed signals and
    querying text, one has to be aware that the text is set to that hint
    text if isShowingHintText() returns true (that is, does not contain
    valid user input).
 */

enum { margin = 6 };

#define ICONBUTTON_HEIGHT 18
#define FADE_TIME 160
#define RIGHT_MARGIN 2
#define LEFT_MARGIN 8

// --------- FancyLineEditPrivate
class FancyLineEditPrivate : public QObject
{
public:
    explicit FancyLineEditPrivate(FancyLineEdit *parent);

    bool eventFilter(QObject *obj, QEvent *event) override;

    FancyLineEdit  *m_lineEdit;
    QPixmap m_pixmap[2];
    QMenu *m_menu[2];
    bool m_menuTabFocusTrigger[2];
    IconButton *m_iconbutton[2];
    bool m_iconEnabled[2];
};


FancyLineEditPrivate::FancyLineEditPrivate(FancyLineEdit *parent) :
    QObject(parent), m_lineEdit(parent)//,  m_historyCompleter(0)
{DD
    for (int i = 0; i < 2; ++i) {
        m_menu[i] = 0;
        m_menuTabFocusTrigger[i] = false;
        m_iconbutton[i] = new IconButton(parent);
        m_iconbutton[i]->installEventFilter(this);
        m_iconbutton[i]->hide();
        m_iconbutton[i]->setAutoHide(false);
        m_iconEnabled[i] = false;
    }
}

bool FancyLineEditPrivate::eventFilter(QObject *obj, QEvent *event)
{
    int buttonIndex = -1;
    for (int i = 0; i < 2; ++i) {
        if (obj == m_iconbutton[i]) {
            buttonIndex = i;
            break;
        }
    }
    if (buttonIndex == -1)
        return QObject::eventFilter(obj, event);
    switch (event->type()) {
    case QEvent::FocusIn:
        if (m_menuTabFocusTrigger[buttonIndex] && m_menu[buttonIndex]) {
            m_lineEdit->setFocus();
            execMenuAtWidget(m_menu[buttonIndex], m_iconbutton[buttonIndex]);
            return true;
        }
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}


// --------- FancyLineEdit
FancyLineEdit::FancyLineEdit(QWidget *parent) :
    QLineEdit(parent),
    d(new FancyLineEditPrivate(this))
{DD
    ensurePolished();
    updateMargins();

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(checkButtons(QString)));
    connect(d->m_iconbutton[Left], SIGNAL(clicked()), this, SLOT(iconClicked()));
    connect(d->m_iconbutton[Right], SIGNAL(clicked()), this, SLOT(iconClicked()));
}

void FancyLineEdit::checkButtons(const QString &text)
{DD
    if (m_oldText.isEmpty() || text.isEmpty()) {
        for (int i = 0; i < 2; ++i) {
            if (d->m_iconbutton[i]->hasAutoHide())
                d->m_iconbutton[i]->animateShow(!text.isEmpty());
        }
        m_oldText = text;
    }
}

FancyLineEdit::~FancyLineEdit()
{DD
}

void FancyLineEdit::setButtonVisible(Side side, bool visible)
{DD
    d->m_iconbutton[side]->setVisible(visible);
    d->m_iconEnabled[side] = visible;
    updateMargins();
}

bool FancyLineEdit::isButtonVisible(Side side) const
{DD
    return d->m_iconEnabled[side];
}

void FancyLineEdit::iconClicked()
{DD
    IconButton *button = qobject_cast<IconButton *>(sender());
    int index = -1;
    for (int i = 0; i < 2; ++i)
        if (d->m_iconbutton[i] == button)
            index = i;
    if (index == -1)
        return;
    if (d->m_menu[index]) {
        execMenuAtWidget(d->m_menu[index], button);
    } else {
        Q_EMIT buttonClicked((Side)index);
        if (index == Left)
            Q_EMIT leftButtonClicked();
        else if (index == Right)
            Q_EMIT rightButtonClicked();
    }
}

void FancyLineEdit::updateMargins()
{DD
    bool leftToRight = (layoutDirection() == Qt::LeftToRight);
    Side realLeft = (leftToRight ? Left : Right);
    Side realRight = (leftToRight ? Right : Left);

    int leftMargin = d->m_iconbutton[realLeft]->pixmap().width();
    leftMargin = leftMargin / d->m_iconbutton[realLeft]->pixmap().devicePixelRatioF() + LEFT_MARGIN;
    int rightMargin = d->m_iconbutton[realRight]->pixmap().width();
    rightMargin = rightMargin / d->m_iconbutton[realRight]->pixmap().devicePixelRatioF() + RIGHT_MARGIN;

    // Note KDE does not reserve space for the highlight color
    if (style()->inherits("OxygenStyle")) {
        leftMargin = qMax(24, leftMargin);
        rightMargin = qMax(24, rightMargin);
    }

    QMargins margins((d->m_iconEnabled[realLeft] ? leftMargin : 0), 0,
                     (d->m_iconEnabled[realRight] ? rightMargin : 0), 0);

    setTextMargins(margins);
}

void FancyLineEdit::updateButtonPositions()
{DD
    QRect contentRect = rect();
    for (int i = 0; i < 2; ++i) {
        Side iconpos = (Side)i;
        if (layoutDirection() == Qt::RightToLeft)
            iconpos = (iconpos == Left ? Right : Left);

        if (iconpos == FancyLineEdit::Right) {
            const int iconoffset = textMargins().right() + 4;
            d->m_iconbutton[i]->setGeometry(contentRect.adjusted(width() - iconoffset, 0, 0, 0));
        } else {
            const int iconoffset = textMargins().left() + 4;
            d->m_iconbutton[i]->setGeometry(contentRect.adjusted(0, 0, -width() + iconoffset, 0));
        }
    }
}

void FancyLineEdit::resizeEvent(QResizeEvent *)
{DD
    updateButtonPositions();
}

void FancyLineEdit::setButtonPixmap(Side side, const QPixmap &buttonPixmap)
{DD;
    d->m_iconbutton[side]->setPixmap(buttonPixmap);
    updateMargins();
    updateButtonPositions();
    update();
}

QPixmap FancyLineEdit::buttonPixmap(Side side) const
{DD;
    return d->m_pixmap[side];
}

void FancyLineEdit::setButtonMenu(Side side, QMenu *buttonMenu)
{DD;
    d->m_menu[side] = buttonMenu;
    d->m_iconbutton[side]->setIconOpacity(1.0);
}

QMenu *FancyLineEdit::buttonMenu(Side side) const
{DD;
    return  d->m_menu[side];
}

bool FancyLineEdit::hasMenuTabFocusTrigger(Side side) const
{DD
    return d->m_menuTabFocusTrigger[side];
}

void FancyLineEdit::setMenuTabFocusTrigger(Side side, bool v)
{DD
    if (d->m_menuTabFocusTrigger[side] == v)
        return;

    d->m_menuTabFocusTrigger[side] = v;
    d->m_iconbutton[side]->setFocusPolicy(v ? Qt::TabFocus : Qt::NoFocus);
}

bool FancyLineEdit::hasAutoHideButton(Side side) const
{DD
    return d->m_iconbutton[side]->hasAutoHide();
}

void FancyLineEdit::setAutoHideButton(Side side, bool h)
{DD
    d->m_iconbutton[side]->setAutoHide(h);
    if (h)
        d->m_iconbutton[side]->setIconOpacity(text().isEmpty() ?  0.0 : 1.0);
    else
        d->m_iconbutton[side]->setIconOpacity(1.0);
}

void FancyLineEdit::setButtonToolTip(Side side, const QString &tip)
{DD
    d->m_iconbutton[side]->setToolTip(tip);
}

void FancyLineEdit::setButtonFocusPolicy(Side side, Qt::FocusPolicy policy)
{DD
    d->m_iconbutton[side]->setFocusPolicy(policy);
}

// IconButton - helper class to represent a clickable icon

IconButton::IconButton(QWidget *parent)
    : QAbstractButton(parent), m_autoHide(false)
{DD
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect pixmapRect = QRect({0, 0}, m_pixmap.size()/m_pixmap.devicePixelRatioF());
    pixmapRect.moveCenter(rect().center());

    if (m_autoHide)
        painter.setOpacity(m_iconOpacity);

    painter.drawPixmap(pixmapRect, m_pixmap);
}

void IconButton::animateShow(bool visible)
{DD
    if (visible) {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "iconOpacity");
        animation->setDuration(FADE_TIME);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "iconOpacity");
        animation->setDuration(FADE_TIME);
        animation->setEndValue(0.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}


SearchLineEdit::SearchLineEdit(QWidget *parent) : FancyLineEdit(parent)
{DD
    QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
                     QSL("edit-clear-locationbar-rtl") :
                     QSL("edit-clear-locationbar-ltr"),
                     QIcon::fromTheme(QSL("edit-clear"), QIcon::fromTheme("editclear")));

    setButtonPixmap(FancyLineEdit::Right, icon.pixmap(SMALL_ICON_SIZE));
    setButtonVisible(FancyLineEdit::Right, true);
    setButtonToolTip(FancyLineEdit::Right, tr("Clear text"));
    setAutoHideButton(FancyLineEdit::Right, true);
    connect(this, SIGNAL(rightButtonClicked()), this, SLOT(clear()));
    setMinimumWidth(200);
    QMenu *lineEditMenu = new QMenu(this);
    setButtonMenu(FancyLineEdit::Left, lineEditMenu);
    setButtonVisible(FancyLineEdit::Left, true);

    static const struct Option {
        const char *text;
        const char *slot;
        const char *icon;
    } options[] = {
        {QT_TR_NOOP("Case Sensitive"),SLOT(setCaseSensitive(bool)),"casesensitively"},
        {QT_TR_NOOP("Whole Words Only"),SLOT(setWholeWord(bool)),"wholewords"},
        {QT_TR_NOOP("Use Regular Expressions"),SLOT(setRegularExpressions(bool)),"regexp"}
    };
    for (int i=0; i<3; ++i) {
        QAction *a = new QAction(options[i].text, this);
        a->setCheckable(true);
        a->setChecked(false);
        a->setIcon(QIcon::fromTheme(options[i].icon));
        connect(a, SIGNAL(triggered(bool)), this, options[i].slot);
        lineEditMenu->addAction(a);
    }

    m_caseSensitive = false;
    m_wholeWord = false;
    m_useRegularExpressions = false;
    findFlagsChanged();
}

void SearchLineEdit::findFlagsChanged()
{DD;
    if (!m_caseSensitive && !m_wholeWord && !m_useRegularExpressions) {
        const QPixmap mag = QIcon::fromTheme("magnifier").pixmap(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
        setButtonPixmap(FancyLineEdit::Left, mag);
    } else {
        setButtonPixmap(FancyLineEdit::Left, pixmapForFindFlags());
    }
}

QPixmap SearchLineEdit::pixmapForFindFlags()
{DD;
    QIcon ico = QIcon::fromTheme("casesensitively");
    static const QPixmap casesensitiveIcon = ico.pixmap(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
    ico = QIcon::fromTheme("regexp");
    static const QPixmap regexpIcon = ico.pixmap(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));
    ico = QIcon::fromTheme("wholewords");
    static const QPixmap wholewordsIcon = ico.pixmap(QSize(SMALL_ICON_SIZE,SMALL_ICON_SIZE));

    int width = 0;
    if (m_caseSensitive) width += 6;
    if (m_wholeWord) width += 6;
    if (m_useRegularExpressions) width += 6;
    if (width > 0) --width;
    QPixmap pixmap(width, SMALL_ICON_SIZE+1);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    int x = 0;

    if (m_caseSensitive) {
        painter.drawPixmap(x - 6, 0, casesensitiveIcon);
        x += 6;
    }
    if (m_wholeWord) {
        painter.drawPixmap(x - 6, 0, wholewordsIcon);
        x += 6;
    }
    if (m_useRegularExpressions) {
        painter.drawPixmap(x - 6, 0, regexpIcon);
    }
    return pixmap;
}

void SearchLineEdit::setCaseSensitive(bool b)
{DD;
    m_caseSensitive = b;
    findFlagsChanged();
}

void SearchLineEdit::setWholeWord(bool b)
{DD;
    m_wholeWord = b;
    findFlagsChanged();
}

void SearchLineEdit::setRegularExpressions(bool b)
{DD;
    m_useRegularExpressions = b;
    findFlagsChanged();
}
