/**
 * \file clearlineedit.cpp
 * There should be a license of Qt Creator.
 * Anyway, this file was borrowed from Qt Creator src,
 * so I don't know whether it is legal or not.
 */

#include "clearlineedit.h"
#include "enums.h"
#include "applicationpaths.h"
#include "application.h"
#include "qoobarglobals.h"
/*
 *A FancyLineEdit with a clear button
 **/
#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)
ClearLineEdit::ClearLineEdit(QWidget *parent) :
    QLineEdit(parent)
{DD;
    this->setClearButtonEnabled(true);
}

#else
ClearLineEdit::ClearLineEdit(QWidget *parent) :
    FancyLineEdit(parent)
{DD;
    QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
                     QSL("edit-clear-locationbar-rtl") :
                     QSL("edit-clear-locationbar-ltr"),
                     QIcon::fromTheme(QSL("edit-clear"), QIcon(App->iconThemeIcon(QSL("editclear.png")))));

    setButtonPixmap(Right, icon.pixmap(16));
    setButtonVisible(Right, true);
    setButtonToolTip(Right, tr("Clear text"));
    setAutoHideButton(Right, true);
    connect(this, SIGNAL(rightButtonClicked()), this, SLOT(clear()));
}
#endif
