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

#ifndef FANCYLINEEDIT_H
#define FANCYLINEEDIT_H

#include <QLineEdit>
#include <QAbstractButton>

class FancyLineEditPrivate;

class IconButton: public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(float iconOpacity READ iconOpacity WRITE setIconOpacity)
    Q_PROPERTY(bool autoHide READ hasAutoHide WRITE setAutoHide)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
public:
    explicit IconButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void setPixmap(const QPixmap &pixmap) { m_pixmap = pixmap; update(); }
    QPixmap pixmap() const { return m_pixmap; }
    float iconOpacity() { return m_iconOpacity; }
    void setIconOpacity(float value) { m_iconOpacity = value; update(); }
    void animateShow(bool visible);

    void setAutoHide(bool hide) { m_autoHide = hide; }
    bool hasAutoHide() const { return m_autoHide; }
private:
    float m_iconOpacity = 0.0;
    bool m_autoHide;
    QPixmap m_pixmap;
};

class FancyLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_ENUMS(Side)

public:
    enum Side {Left = 0, Right = 1};

    explicit FancyLineEdit(QWidget *parent = 0);
    ~FancyLineEdit();

    QPixmap buttonPixmap(Side side) const;
    void setButtonPixmap(Side side, const QPixmap &pixmap);

    QMenu *buttonMenu(Side side) const;
    void setButtonMenu(Side side, QMenu *menu);

    void setButtonVisible(Side side, bool visible);
    bool isButtonVisible(Side side) const;

    void setButtonToolTip(Side side, const QString &);
    void setButtonFocusPolicy(Side side, Qt::FocusPolicy policy);

    // Set whether tabbing in will trigger the menu.
    void setMenuTabFocusTrigger(Side side, bool v);
    bool hasMenuTabFocusTrigger(Side side) const;

    // Set if icon should be hidden when text is empty
    void setAutoHideButton(Side side, bool h);
    bool hasAutoHideButton(Side side) const;

Q_SIGNALS:
    void buttonClicked(FancyLineEdit::Side side);
    void leftButtonClicked();
    void rightButtonClicked();

private Q_SLOTS:
    void checkButtons(const QString &);
    void iconClicked();

protected:
    virtual void resizeEvent(QResizeEvent *e);

private:
    void updateMargins();
    void updateButtonPositions();
    friend class FancyLineEditPrivate;

    FancyLineEditPrivate *d;
    QString m_oldText;
};

class SearchLineEdit : public FancyLineEdit
{
    Q_OBJECT
public:
    explicit SearchLineEdit(QWidget *parent = 0);
    bool caseSensitive() const {return m_caseSensitive;}
    bool wholeWord() const {return m_wholeWord;}
    bool useRegularExpressions() const {return m_useRegularExpressions;}
private Q_SLOTS:
    void setCaseSensitive(bool b);
    void setWholeWord(bool b);
    void setRegularExpressions(bool b);
private:
    void findFlagsChanged();
    QPixmap pixmapForFindFlags();

    bool m_caseSensitive;
    bool m_wholeWord;
    bool m_useRegularExpressions;
};

#endif // FANCYLINEEDIT_H
