#ifndef IQOOBARPLUGIN_H
#define IQOOBARPLUGIN_H

#include <QStringList>
#include <QIcon>
#include "tagger.h"

class IQoobarPlugin
{
public:
    virtual ~IQoobarPlugin() {}

    /*does all editing*/
    virtual QList<Tag> getNewTags(const QList<Tag> &oldTags) = 0;

    /*returns true if plugin can work with no files selected
     * (f.e. adding files */
    virtual bool canWorkWithNoFilesSelected() = 0;
};

Q_DECLARE_INTERFACE(IQoobarPlugin, "qoobar.IQoobarPlugin/1.1")

#endif // IQOOBARPLUGIN_H
