/**
 * \file taggingscheme.cpp
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

#include "taggingscheme.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "qoobarglobals.h"
#include <QFileInfo>
#include "applicationpaths.h"

#include <QtDebug>

#include "enums.h"

TaggingScheme::TaggingScheme(const QString &filePath)
    : m_filePath(filePath)
{DD;
//    m_fieldsForParsing.resize(TAGSCOUNT);
//    m_fields.resize(TAGSCOUNT);
    nav.resize(5);
    changed = false;
}

void TaggingScheme::setFilePath(const QString &filePath)
{DD;
    m_filePath = filePath;
}

void TaggingScheme::read(bool nameOnly)
{DD;
    QFile f(m_filePath);
    if (!f.exists()) {
        f.setFileName(QSL(":/src/default.xml"));
    }
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return;

    QXmlStreamReader xml(&f);
    TagType type = UNKNOWN;

    while (!xml.atEnd()) {
        xml.readNext();
        auto name=xml.name().toString();
        if (xml.isStartElement()) {
            if (name.compare(QSL("scheme"))==0) {
                setName(xml.attributes().value(QSL("name")).toString());
                if (nameOnly) return;
            }
            else if (name.compare(QSL("tag"))==0) {
                addField(xml.attributes().value(QSL("name")).toString(),
                         xml.attributes().value(QSL("id")).toString().toInt());
                if (!xml.attributes().hasAttribute(QSL("canBeSimplified"))) {
                    //fixing old scheme files
                    if (m_fields.last().id == LYRICS ||
                        m_fields.last().id == COMMENT ||
                        m_fields.last().name==QSL("Synchronized lyrics"))
                        m_fields.last().canBeSimplified = false;
                    else
                        m_fields.last().canBeSimplified = true;
                }
                else
                    m_fields.last().canBeSimplified = xml.attributes().value(QSL("canBeSimplified")).toString()==QSL("true");
            }
            else if (name.compare(QSL("format"))==0) {
                type = (TagType)xml.attributes().value(QSL("id")).toString().toInt();
            }
            else if (name.compare(QSL("field"))==0) {
                if (type!=UNKNOWN) {
                    QString s = xml.readElementText();
                    setFields(m_fields.size()-1, type, QStringList()<<s);
                }
            }
        }
    }
    changed = false;
    retranslateUI();
}

void TaggingScheme::retranslateUI()
{DD;
    localizedFieldName.resize(tagsCount());
    for (int i=0; i<TAGSCOUNT; ++i) localizedFieldName[i] = Qoobar::tagByID(i);
    for (int i=TAGSCOUNT; i<tagsCount(); ++i) localizedFieldName[i] = fieldName(i);
}

bool TaggingScheme::save()
{DD;
    //try to save into user settings folder
    m_filePath = QString("%1/%2").arg(ApplicationPaths::userSchemesPath())
            .arg(QFileInfo(m_filePath).fileName());

    bool saved = false;
    QFile f(m_filePath);
    if (f.open(QFile::WriteOnly)) {
        QXmlStreamWriter stream(&f);
        stream.setAutoFormatting(true);
        stream.writeStartDocument(QSL("1.0"));

        stream.writeStartElement(QSL("scheme"));
        stream.writeAttribute(QSL("name"), m_name);
        Q_FOREACH (FieldDescription field, m_fields) {
            stream.writeStartElement(QSL("tag"));
            stream.writeAttribute(QSL("name"), field.name);
            stream.writeAttribute(QSL("id"), QString::number(field.id));
            stream.writeAttribute(QSL("canBeSimplified"), field.canBeSimplified?QSL("true"):QSL("false"));

            for (int j=0; j<field.fields.size(); ++j) {
                stream.writeStartElement(QSL("format"));
                stream.writeAttribute(QSL("id"), QString::number(j));
                QStringList fieldsList = field.fields.at(j);

                Q_FOREACH (const QString &field, fieldsList) {
                    stream.writeTextElement(QSL("field"), field);
                }
                stream.writeEndElement(); //"format"
            }
            stream.writeEndElement(); //"tag"
        }
        stream.writeEndDocument();
        saved = true;
        changed = false;
    }
    return saved;
}

bool TaggingScheme::operator==(const TaggingScheme &other) const
{DD;
    if (tagsCount() != other.tagsCount()) return false;
    return (m_fields == other.m_fields);
}

QList<int> TaggingScheme::search(TagType tagType, const QString &field)
{DD;
    return nav.at(int(tagType)).value(field);
}

int TaggingScheme::tagsCount() const
{DD;
    return m_fields.size();
}

//bool TaggingScheme::equal(TagType tagType,const QString &field, int id) const
//{DD;
//    QMultiHash<QString, int> hash=m_fieldsForParsing.at(tagType);
//    return (hash.contains(field, id) && hash.contains(field, id+1));

//    //QVector<int> fieldsIDs = m_fieldsForParsing.value(tagType).values(field);
//    ///return (fieldsIDs.contains(id) && fieldsIDs.contains(id+1));
//}

QStringList TaggingScheme::fields(int tagID, TagType tagType) const
{DD;
    return m_fields.at(tagID).fields.at(tagType);
}

void TaggingScheme::clear()
{DD;
    m_fields.clear();
    nav.clear();
    nav.resize(5);
    changed = false;
}

void TaggingScheme::setFields(int tagID, TagType tagType, const QStringList &fields)
{DD;
//    QStringList f = m_fields[tagID].fields[tagType];
    m_fields[tagID].fields[tagType].append(fields);

    Q_FOREACH (const QString &s, fields) {
        QList<int> indexes = nav[tagType].value(s);
        indexes << tagID;
        nav[tagType].insert(s, indexes);
    }
}

QString TaggingScheme::name() const
{DD;
    return m_name;
}

void TaggingScheme::setName(const QString &name)
{DD;
    m_name=name;
}

QString TaggingScheme::fieldName(int tagID) const
{DD;
    return m_fields.value(tagID).name;
}

bool TaggingScheme::canBeSimplified(int tagID) const
{
    return m_fields.at(tagID).canBeSimplified;

//    if (tagID == LYRICS || tagID == COMMENT || fieldName(tagID)=="Synchronized lyrics") return false;
//    return true;
}

//QString TaggingScheme::localizedFieldName(int tagID) const
//{DD;
//    return (tagID < TAGSCOUNT ? Qoobar::tagByID(tagID) : fieldName(tagID));
//}

int TaggingScheme::tagIDByName(const QString &name) const
{DD;
    Q_FOREACH(const FieldDescription &descr, m_fields) {
        if (descr.name == name) return descr.id;
    }
    return -1;
}

int TaggingScheme::tagIDBySimplifiedName(const QString &name) const
{DD;
    const QString simplifiedName = name.toLower().remove(QSL(" "));
    Q_FOREACH(const FieldDescription &descr, m_fields) {
        if (descr.name.toLower().remove(QSL(" ")) == simplifiedName) return descr.id;
        if (descr.id < TAGSCOUNT) {//search also by localized tag name
            if (Qoobar::tagByID(descr.id).toLower().remove(QSL(" ")) == simplifiedName) return descr.id;
        }
    }
    return -1;
}

void TaggingScheme::addField(const QString &fieldName, int id)
{DD;
    FieldDescription field;
    field.fields.resize(5);
    field.id = id;
    field.name = fieldName;
    m_fields << field;
    changed = true;
}
