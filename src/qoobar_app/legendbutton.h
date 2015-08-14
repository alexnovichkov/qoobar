/**
 * \file legendbutton.h
 * A button with drop-down menu and some signals
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 11 Aug 2011
 *
 * Copyright (C) 2011  Alex Novichkov
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

#ifndef LEGENDBUTTON_H
#define LEGENDBUTTON_H

#include <QPushButton>



class LegendButton : public QPushButton
{
    Q_OBJECT
public:
    enum PlaceholderCategory {
        NoPlaceholders = 0x0,
        WritablePlaceholders = 0x1,
        ReadOnlyPlaceholders = 0x2,
        VoidPlaceholder = 0x4
    };
    Q_DECLARE_FLAGS(PlaceholderCategories, PlaceholderCategory)

    explicit LegendButton(QWidget *parent = 0);
    void setCategories(PlaceholderCategories categories) {this->categories = categories;}
    void retranslateUi();
Q_SIGNALS:
    void placeholderChosen(const QString &);
private Q_SLOTS:
    void actionTriggered(QAction *);
protected:
    bool event(QEvent *);
private:
    PlaceholderCategories categories;
    QMenu *menu;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LegendButton::PlaceholderCategories)

#endif // LEGENDBUTTON_H
