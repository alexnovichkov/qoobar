/**
 * \file clearlineedit.h
 * There should be a license of Qt Creator.
 * Anyway, this file was borrowed from Qt Creator src,
 * so I don't know whether it is legal or not.
 */

#ifndef CLEARLINEEDIT_H
#define CLEARLINEEDIT_H

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5,2,0))
#include <QLineEdit>

class ClearLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ClearLineEdit(QWidget *parent = 0);
};

#else

#include "fancylineedit.h"
class ClearLineEdit : public FancyLineEdit
{
//#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
//    Q_OBJECT
//#endif
public:
    explicit ClearLineEdit(QWidget *parent = 0);
};
#endif

#endif // CLEARLINEEDIT_H
