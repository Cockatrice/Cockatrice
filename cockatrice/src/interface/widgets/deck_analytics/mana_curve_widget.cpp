#include "mana_curve_widget.h"

#include "../../../main.h"
#include "../../deck_loader/deck_loader.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <unordered_map>

ManaCurveWidget::ManaCurveWidget(QWidget *parent, DeckListStatisticsAnalyzer *_deckStatAnalyzer)
    : QWidget(parent), deckStatAnalyzer(_deckStatAnalyzer)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Curve"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barContainer = new QWidget(this);
    barLayout = new QHBoxLayout(barContainer);
    layout->addWidget(barContainer);

    connect(deckStatAnalyzer, &DeckListStatisticsAnalyzer::statsUpdated, this, &ManaCurveWidget::updateDisplay);

    retranslateUi();
}

void ManaCurveWidget::retranslateUi()
{
    bannerWidget->setText(tr("Mana Curve"));
}

void ManaCurveWidget::updateDisplay()
{
    // Clear the layout first
    if (barLayout != nullptr) {
        QLayoutItem *item;
        while ((item = barLayout->takeAt(0)) != nullptr) {
            item->widget()->deleteLater();
            delete item;
        }
    }

    auto manaCurveMap = deckStatAnalyzer->getManaCurve();

    int highestEntry = 0;
    for (const auto &entry : manaCurveMap) {
        if (entry.second > highestEntry) {
            highestEntry = entry.second;
        }
    }

    // Convert unordered_map to ordered map to ensure sorting by CMC
    std::map<int, int> sortedManaCurve(manaCurveMap.begin(), manaCurveMap.end());

    // Add new widgets to the layout in sorted order
    for (const auto &entry : sortedManaCurve) {
        BarWidget *barWidget =
            new BarWidget(QString::number(entry.first), entry.second, highestEntry, QColor(122, 122, 122), this);
        barLayout->addWidget(barWidget);
    }

    update(); // Update the widget display
}
