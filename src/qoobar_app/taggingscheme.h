/**
 * \file taggingscheme.h
 * Class for reading, writing and storing tag manipulation scheme.
 *
 * \b Project: Qoobar
 * \author Alex Novichkov
 * \date 17 Jan 2012
 *
 * Copyright (C) 2012  Alex Novichkov
 *
 * This file is part of Qoobar.
 *
 * Qoobar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef TAGGINGSCHEME_H
#define TAGGINGSCHEME_H

#include <QStringList>
#include <QHash>
#include <QVector>

class FieldDescription
{
public:
    int id;
    QString name;
//    QChar placeholder;
    QVector<QStringList> fields;
    bool operator ==(const FieldDescription &other) const
    {
        return (other.id == id && other.name == name && other.fields == fields);
    }
};



class TaggingScheme
{
public:
    enum TagType {
        UNKNOWN=-1,
        ID3=0,
        APE=1,
        VORBIS=2,
        MP4=3,
        ASF=4
    };

    TaggingScheme(const QString &filePath);
    void setFilePath(const QString &filePath);
    inline QString filePath() const {return m_filePath;}
    void read(bool nameOnly=false);

    void retranslateUI();

    QStringList fields(int tagID, TagType tagType) const;
    void setFields(int tagID, TagType tagType, const QStringList &fields);
    void clear();
    QString name() const;
    void setName(const QString &);

    QString fieldName(int tagID) const;
    bool canBeSimplified(int tagID) const;
//    QString localizedFieldName(int tagID) const;

    int tagIDByName(const QString &name) const;

    int tagIDBySimplifiedName(const QString &name) const;

    void addField(const QString fieldName, int id);

    QList<int> search(TagType tagType, const QString &field);

    int tagsCount() const;

//    bool equal(TagType tagType,const QString &field,int tagID) const;
    bool save();

    bool wasChanged() const {return changed;}

    bool operator==(const TaggingScheme &other) const;

    QVector<QString> localizedFieldName;
private:
//    QVector<QMultiHash<QString, int> > m_fieldsForParsing;
//    QVector<QMultiHash<int, QString> > m_fields;
    QVector<FieldDescription> m_fields;

    QVector<QHash<QString, QList<int> > > nav;
    QString m_name;
    QString m_filePath;
    bool changed;
};

#endif // TAGGINGSCHEME_H
