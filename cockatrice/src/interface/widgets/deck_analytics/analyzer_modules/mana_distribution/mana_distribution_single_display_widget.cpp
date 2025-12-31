#include "mana_distribution_single_display_widget.h"

#include "../../../cards/additional_info/mana_symbol_widget.h"

#include <QVBoxLayout>

ManaDistributionSingleDisplayWidget::ManaDistributionSingleDisplayWidget(const QString &colorSymbol, QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);

    symbolLabel = new ManaSymbolWidget(this, colorSymbol, true, false);
    symbolLabel->setFixedSize(40, 40);

    devotionBar = new QProgressBar(this);
    devotionBar->setRange(0, 100);
    devotionBar->setTextVisible(false);

    devotionLabel = new QLabel(this);
    devotionLabel->setAlignment(Qt::AlignCenter);

    productionBar = new QProgressBar(this);
    productionBar->setRange(0, 100);
    productionBar->setTextVisible(false);

    productionLabel = new QLabel(this);
    productionLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(symbolLabel);
    layout->addWidget(devotionBar);
    layout->addWidget(devotionLabel);
    layout->addWidget(productionBar);
    layout->addWidget(productionLabel);

    setLayout(layout);
}

void ManaDistributionSingleDisplayWidget::setDevotion(int pips, int cards, int percent)
{
    devotionBar->setValue(percent);
    devotionLabel->setText(QString(tr("%1 pips (%2 cards)")).arg(pips).arg(cards));
}

void ManaDistributionSingleDisplayWidget::setProduction(int pips, int cards, int percent)
{
    productionBar->setValue(percent);
    productionLabel->setText(QString(tr("%1 mana (%2 cards)")).arg(pips).arg(cards));
}
