#ifndef FILENAMERENDERER_H
#define FILENAMERENDERER_H

#include "tagsrenderer.h"

class FileNameRenderer: public TagsRenderer
{
    Q_OBJECT
public:
    FileNameRenderer(QObject *parent=0): TagsRenderer(parent) {}
    FileNameRenderer(Model *model, QObject *parent=0)
        : TagsRenderer(model, parent) {}
    QStringList getPrefixes() const {return prefixes;}
private:
    virtual void updateTags();
    virtual void preprocess();
    QStringList prefixes;
};

#endif // FILENAMERENDERER_H
