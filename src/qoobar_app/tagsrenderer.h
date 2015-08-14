/**
 * \file tagrender.h
 * Renders tags from placeholders.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 31 Dec 2010
 *
 * Copyright (C) 2010  Alex Novichkov
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

#ifndef TAGRENDER_H
#define TAGRENDER_H

#include <QStringList>
#include <QObject>
class Model;

class TagsRenderer: public QObject
{
Q_OBJECT
    Q_PROPERTY(QString pattern READ pattern WRITE setPattern NOTIFY patternChanged)
    Q_PROPERTY(QStringList newTags READ newTags)
public:
    TagsRenderer(QObject *parent=0);
    TagsRenderer(Model *model, QObject *parent=0);
    virtual ~TagsRenderer() {}

    QString pattern() const {return m_pattern;}
    QStringList newTags() {updateTags(); return m_newTags;}

    virtual void updateTags();
    virtual void preprocess();
    virtual void postprocess();
Q_SIGNALS:
    void patternChanged(const QString &);
public Q_SLOTS:
    bool setPattern(const QString &pattern);
protected:
    QString m_pattern;
    QStringList m_newTags;
    Model *m;
};

#endif // TAGRENDER_H
