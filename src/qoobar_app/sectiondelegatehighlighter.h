#ifndef SECTIONDELEGATEHIGHLIGHTER_H
#define SECTIONDELEGATEHIGHLIGHTER_H

#include "delegatehighlighter.h"

class SectionDelegateHighlighter : public DelegateHighlighter
{
public:
    SectionDelegateHighlighter();
    QList<QTextLayout::FormatRange> generateFormats(
        #if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
            QStyleOptionViewItemV4
        #else
            QStyleOptionViewItem
        #endif
            *, const QRect &, const QModelIndex &);
};

#endif // SECTIONDELEGATEHIGHLIGHTER_H
