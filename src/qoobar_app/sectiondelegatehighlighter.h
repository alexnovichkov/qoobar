#ifndef SECTIONDELEGATEHIGHLIGHTER_H
#define SECTIONDELEGATEHIGHLIGHTER_H

#include "delegatehighlighter.h"

class SectionDelegateHighlighter : public DelegateHighlighter
{
public:
    SectionDelegateHighlighter();
    QList<QTextLayout::FormatRange> generateFormats(QStyleOptionViewItemV4 *, const QRect &, const QModelIndex &);
};

#endif // SECTIONDELEGATEHIGHLIGHTER_H
