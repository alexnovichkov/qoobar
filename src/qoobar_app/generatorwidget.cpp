#include "generatorwidget.h"

#include <QtWidgets>

#include "application.h"
#include "legendbutton.h"
#include "headerview.h"

#include "enums.h"
#include "qoobarglobals.h"
#include "logging.h"
#include "importmodel.h"

GeneratorWidget::GeneratorWidget(const QList<Tag> &oldTags, QWidget *parent) : QWidget(parent)
{DD;
    QLabel *patternLabel = new QLabel(tr("Pattern:"),this);

    tagsSourceComboBox = new QComboBox(this);
    tagsSourceComboBox->setEditable(false);
    tagsSourceComboBox->addItems({tr("File name"), tr("Clipboard")});
    const int tagsCount = App->currentScheme->tagsCount();
    for (int i=0; i<tagsCount; ++i)
        tagsSourceComboBox->addItem(App->currentScheme->localizedFieldName[i]);
    tagsSourceComboBox->adjustSize();
    connect(tagsSourceComboBox,SIGNAL(currentIndexChanged(int)),SLOT(setSource(int)));

    patternEdit = new QComboBox(this);
    patternEdit->setSizePolicy(QSizePolicy::Expanding, patternEdit->sizePolicy().verticalPolicy());
    patternEdit->setEditable(true);
    patternEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);

    if (!App->fillPatterns.isEmpty()) {
        patternEdit->insertItems(0,App->fillPatterns);
        patternEdit->setEditText(App->fillPatterns.first());
    }
    connect(patternEdit,SIGNAL(currentTextChanged(QString)),this, SLOT(updateTags()));
    connect(patternEdit,SIGNAL(editTextChanged(QString)),this, SLOT(updateTags()));

    legendButton = new LegendButton(this);
    legendButton->setCategories(LegendButton::WritablePlaceholders | LegendButton::VoidPlaceholder);
    connect(legendButton, &LegendButton::placeholderChosen, [=](const QString &pattern){
        patternEdit->lineEdit()->insert(pattern);
        patternEdit->setFocus();
    });
    legendButton->setFocusPolicy(Qt::NoFocus);
    legendButton->retranslateUi();

    table = new QTableView(this);

#ifdef OSX_SUPPORT_ENABLED
    table->setAttribute(Qt::WA_MacSmallSize, true);
    table->setWordWrap(true);
#else
    table->setWordWrap(false);
#endif
    importModel = new ImportModel(table);
    importModel->setTags(oldTags);
    table->setModel(importModel);

    header = new HeaderView(Qt::Horizontal,table);
    table->setHorizontalHeader(header);
    table->setColumnWidth(0,25);
    table->setColumnWidth(1,400);
    table->resizeColumnToContents(0);

    setSource(0);
    updateTags();

    QHBoxLayout *patternLayout = new QHBoxLayout;
    patternLayout->addWidget(new QLabel(tr("Source:"),this));
    patternLayout->addWidget(tagsSourceComboBox);
    patternLayout->addWidget(patternLabel);
    patternLayout->addWidget(patternEdit);
    patternLayout->addWidget(legendButton);

    QVBoxLayout *flayout = new QVBoxLayout;
    flayout->addLayout(patternLayout);
    flayout->addWidget(table);
    setLayout(flayout);
}

void GeneratorWidget::updateTags()
{DD;
    importModel->setPattern(patternEdit->currentText());
}

void GeneratorWidget::setSource(int sourceId)
{DD;
    if (sourceId < 0) return;
    importModel->setSource(sourceId);
}

QList<Tag> GeneratorWidget::getTags() const
{DD;
    App->addPattern(patternEdit->currentText(), App->fillPatterns);
    return importModel->getTags();
}
