#ifndef HIGHLIGHTDELEGATE_H
#define HIGHLIGHTDELEGATE_H

#include <QStyledItemDelegate>
class DelegateHighlighter;
class QTextLayout;

class HighlightDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit HighlightDelegate(QObject *parent = 0);
    ~HighlightDelegate();
    void setHighlighter(DelegateHighlighter *highlighter);
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
private:
    QTextLayout *textLayout;
    DelegateHighlighter *highlighter;
};

#endif // HIGHLIGHTDELEGATE_H
