#include "mana_devotion_widget.h"

#include "../../deck_loader/deck_loader.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <regex>
#include <unordered_map>

ManaDevotionWidget::ManaDevotionWidget(QWidget *parent, DeckListStatisticsAnalyzer *_deckStatAnalyzer)
    : QWidget(parent), deckStatAnalyzer(_deckStatAnalyzer)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Devotion"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barLayout = new QHBoxLayout();
    layout->addLayout(barLayout);

    connect(deckStatAnalyzer, &DeckListStatisticsAnalyzer::statsUpdated, this, &ManaDevotionWidget::updateDisplay);

    retranslateUi();
}

void ManaDevotionWidget::retranslateUi()
{
    bannerWidget->setText(tr("Mana Devotion"));
}

void ManaDevotionWidget::updateDisplay()
{
    // Clear the layout first
    QLayoutItem *item;
    while ((item = barLayout->takeAt(0)) != nullptr) {
        item->widget()->deleteLater();
        delete item;
    }

    auto manaDevotionMap = deckStatAnalyzer->getDevotion();

    int highestEntry = 0;
    for (auto entry : manaDevotionMap) {
        if (highestEntry < entry.second) {
            highestEntry = entry.second;
        }
    }

    // Define color mapping for devotion bars
    std::unordered_map<char, QColor> manaColors = {{'W', QColor(248, 231, 185)}, {'U', QColor(14, 104, 171)},
                                                   {'B', QColor(21, 11, 0)},     {'R', QColor(211, 32, 42)},
                                                   {'G', QColor(0, 115, 62)},    {'C', QColor(150, 150, 150)}};

    for (auto entry : manaDevotionMap) {
        QColor barColor = manaColors.count(entry.first) ? manaColors[entry.first] : Qt::gray;
        BarWidget *barWidget = new BarWidget(QString(entry.first), entry.second, highestEntry, barColor, this);
        barLayout->addWidget(barWidget);
    }

    update(); // Update the widget display
}
