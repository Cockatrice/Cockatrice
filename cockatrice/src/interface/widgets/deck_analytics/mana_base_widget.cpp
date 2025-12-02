#include "mana_base_widget.h"

#include "../../deck_loader/deck_loader.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <QHash>
#include <QRegularExpression>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>

ManaBaseWidget::ManaBaseWidget(QWidget *parent, DeckListStatisticsAnalyzer *_deckStatAnalyzer)
    : QWidget(parent), deckStatAnalyzer(_deckStatAnalyzer)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Base"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barContainer = new QWidget(this);
    barLayout = new QHBoxLayout(barContainer);
    layout->addWidget(barContainer);

    connect(deckStatAnalyzer, &DeckListStatisticsAnalyzer::statsUpdated, this, &ManaBaseWidget::updateDisplay);

    retranslateUi();
}

void ManaBaseWidget::retranslateUi()
{
    bannerWidget->setText(tr("Mana Base"));
}

void ManaBaseWidget::updateDisplay()
{
    // Clear the layout first
    QLayoutItem *item;
    while ((item = barLayout->takeAt(0)) != nullptr) {
        item->widget()->deleteLater();
        delete item;
    }

    auto manaBaseMap = deckStatAnalyzer->getManaBase();

    int highestEntry = 0;
    for (auto entry : manaBaseMap) {
        if (entry > highestEntry) {
            highestEntry = entry;
        }
    }

    // Define color mapping for mana types
    QHash<QString, QColor> manaColors;
    manaColors.insert("W", QColor(248, 231, 185));
    manaColors.insert("U", QColor(14, 104, 171));
    manaColors.insert("B", QColor(21, 11, 0));
    manaColors.insert("R", QColor(211, 32, 42));
    manaColors.insert("G", QColor(0, 115, 62));
    manaColors.insert("C", QColor(150, 150, 150));

    for (auto manaColor : manaBaseMap.keys()) {
        QColor barColor = manaColors.value(manaColor, Qt::gray);
        BarWidget *barWidget = new BarWidget(QString(manaColor), manaBaseMap[manaColor], highestEntry, barColor, this);
        barLayout->addWidget(barWidget);
    }

    update();
}
