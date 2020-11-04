#include "highlightdelegate.h"
#include "delegatehighlighter.h"
#include <QtWidgets>
#include "qoobarglobals.h"

HighlightDelegate::HighlightDelegate(QObject *parent) :
    QStyledItemDelegate(parent), highlighter(0)

{DD;
    textLayout = new QTextLayout();
}

HighlightDelegate::~HighlightDelegate()
{DD;

}

void HighlightDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!textLayout || !highlighter || (option.state & QStyle::State_Selected)) {
        QStyledItemDelegate::paint(painter,option,index);
        return;
    }

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QStyleOptionViewItemV4
#else
    QStyleOptionViewItem
#endif
            optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);

    QList<QTextLayout::FormatRange> formats=highlighter->generateFormats(&optionV4,textRect,index);

    textLayout->setText(optionV4.text);
#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
    textLayout.setAdditionalFormats(formats);
#else
    textLayout->setFormats(formats.toVector());
#endif

    QTextOption textOption = textLayout->textOption();
    textOption.setWrapMode(QTextOption::NoWrap);
    textLayout->setTextOption(textOption);

    textLayout->beginLayout();
    QTextLine line = textLayout->createLine();
    if (line.isValid()) {
        int indent=3;
        line.setLineWidth(textRect.width()-2*indent);
        qreal vertPos=line.height();
        vertPos = vertPos <= textRect.height()?(textRect.height()-vertPos)/2.0 : 0.0;
        line.setPosition(QPointF(1,vertPos));
    }
    textLayout->endLayout();
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    textLayout->draw(painter,QPointF(0,0));
    painter->restore();
}

void HighlightDelegate::setHighlighter(DelegateHighlighter *highlighter)
{DD;
    this->highlighter = highlighter;
}
