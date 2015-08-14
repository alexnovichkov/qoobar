#ifndef IQOOBARPLUGIN_H
#define IQOOBARPLUGIN_H

#include <QStringList>
#include <QIcon>
#include "tagger.h"

class IQoobarPlugin
{
public:
    virtual ~IQoobarPlugin() {}

#ifndef HAVE_QT5
    /*returns localized text for menus, tollbars etc.*/
    virtual QString text() = 0;

    /*returns a full localized description of a plugin*/
    virtual QString description() = 0;

    /*returns a short unique key that identifies a plugin*/
    virtual QString key() = 0;

    /*returns an icon of a plugin*/
    virtual QIcon icon() = 0;

    /*returns a version string*/
    virtual QString version() = 0;
#endif
    /*does all editing*/
    virtual QList<Tag> getNewTags(const QList<Tag> &oldTags) = 0;

    /*returns true if plugin can work with no files selected
     * (f.e. adding files */
    virtual bool canWorkWithNoFilesSelected() = 0;
};

Q_DECLARE_INTERFACE(IQoobarPlugin, "qoobar.IQoobarPlugin/1.1")

#endif // IQOOBARPLUGIN_H
