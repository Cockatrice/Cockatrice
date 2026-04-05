#include "tally_widget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QScrollArea>

TallyWidget::TallyWidget(QWidget *parent, TallyManager *tallyManager) : QWidget(parent), tallyManager(tallyManager)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    auto scrollArea = new QScrollArea(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(scrollArea);

    auto gridLayoutWidget = new QWidget(scrollArea);
    gridLayout = new QGridLayout(gridLayoutWidget);
    gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    resetGridLayout({TallyManager::TOTAL_POWER, TallyManager::TOTAL_MANA_VALUE});
}

void TallyWidget::resetGridLayout(QList<TallyManager::EntryType> entryTypes)
{
    // clear all existing
    while (auto item = gridLayout->takeAt(0)) {
        item->widget()->deleteLater();
        delete item;
    }

    // create all labels
    for (auto entryType : entryTypes) {
        auto textLabel = createTextLabel(entryType);
        auto valueLabel = createValueLabel(entryType);

        int row = gridLayout->rowCount();
        gridLayout->addWidget(textLabel, row, 0);
        gridLayout->addWidget(valueLabel, row, 1);
    }
}

QLabel *TallyWidget::createTextLabel(TallyManager::EntryType entryType)
{
    auto label = new QLabel;

    auto updateLabel = [label, entryType](const QMap<TallyManager::EntryType, TallyResult> &results) {
        QString text = results.value(entryType).text;
        label->setText(text);
    };

    updateLabel(tallyManager->getAllResults());

    connect(tallyManager, &TallyManager::tallyChanged, label, updateLabel);

    return label;
}

QLabel *TallyWidget::createValueLabel(TallyManager::EntryType entryType)
{
    auto label = new QLabel;

    auto updateLabel = [label, entryType](const QMap<TallyManager::EntryType, TallyResult> &results) {
        int value = results.value(entryType).value;
        label->setText(QString::number(value));
    };

    updateLabel(tallyManager->getAllResults());

    connect(tallyManager, &TallyManager::tallyChanged, label, updateLabel);

    return label;
}
