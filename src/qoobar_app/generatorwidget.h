#ifndef GENERATORWIDGET_H
#define GENERATORWIDGET_H

#include <QWidget>

#include "tagger.h"

class QComboBox;
class QTableView;
class ImportModel;
class LegendButton;
class HeaderView;

class GeneratorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GeneratorWidget(const QList<Tag> &oldTags, QWidget *parent = nullptr);
    QList<Tag> getTags() const;
private Q_SLOTS:
    void updateTags();
    void setSource(int sourceId);
private:
    QComboBox *tagsSourceComboBox;
    QComboBox *patternEdit;
    QTableView *table;
    ImportModel *importModel;
    LegendButton *legendButton;
    QStringList tagsSource;
    HeaderView *header;
};

#endif // GENERATORWIDGET_H
