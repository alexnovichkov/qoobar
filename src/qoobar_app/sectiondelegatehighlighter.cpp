#include "sectiondelegatehighlighter.h"
#include "qoobarglobals.h"

SectionDelegateHighlighter::SectionDelegateHighlighter()
{DD
}

QList<QTextLayout::FormatRange> SectionDelegateHighlighter::generateFormats(
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
        QStyleOptionViewItemV4
#else
        QStyleOptionViewItem
#endif
        *option,
        const QRect &textRect,
        const QModelIndex &index)
{DD
    QList<QTextLayout::FormatRange> formats;

    QString text = index.data().toString();
    QString elidedText = option->fontMetrics.elidedText(text, Qt::ElideRight, textRect.width());
    QPoint p=index.data(Qt::UserRole+1).toPoint();
    int start = p.x();
    int length = p.y();

    if (start>=0 && length>0) {
        formats << formatRange(0,start,charFormat(option->font));
        formats << formatRange(start,length, selectionFormat(option->font));
        formats << formatRange(start+length,elidedText.length()-start-length,charFormat(option->font));
    }
    else
        formats << formatRange(0,elidedText.length(),charFormat(option->font));
    option->text = elidedText;

    return formats;
}
