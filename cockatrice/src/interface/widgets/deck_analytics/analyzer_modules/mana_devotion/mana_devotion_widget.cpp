#include "mana_devotion_widget.h"

#include "../../../general/display/charts/bars/bar_widget.h"
#include "../../../general/display/charts/bars/color_bar.h"
#include "../../../general/display/charts/pies/color_pie.h"
#include "../../analytics_panel_widget_registrar.h"
#include "../../deck_list_statistics_analyzer.h"
#include "mana_devotion_config_dialog.h"

#include <QHash>
#include <QInputDialog>

namespace
{

AnalyticsPanelWidgetRegistrar registerManaDevotion{
    "manaDevotion", ManaDevotionWidget::tr("Mana Devotion"),
    [](QWidget *parent, DeckListStatisticsAnalyzer *analyzer) { return new ManaDevotionWidget(parent, analyzer); }};

} // anonymous namespace

ManaDevotionWidget::ManaDevotionWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer, ManaDevotionConfig cfg)
    : AbstractAnalyticsPanelWidget(parent, analyzer), config(std::move(cfg))
{
    barContainer = new QWidget(this);
    barLayout = new QHBoxLayout(barContainer);
    barContainer->setLayout(barLayout);
    layout->addWidget(barContainer);

    updateDisplay();
}

void ManaDevotionWidget::updateDisplay()
{
    // Clear previous widgets
    while (QLayoutItem *item = barLayout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    auto &pipCount = analyzer->getDevotionPipCount();
    auto &cardCount = analyzer->getDevotionCardCount();

    // Convert keys to single QChar form
    QHash<QChar, int> devoMap;
    for (auto key : pipCount.keys()) {
        devoMap[key[0]] = pipCount[key];
    }

    // Apply filters
    if (!config.filters.isEmpty()) {
        QHash<QChar, int> filtered;
        for (auto f : config.filters) {
            if (devoMap.contains(f[0])) {
                filtered[f[0]] = devoMap[f[0]];
            }
        }
        devoMap = filtered;
    }

    // Determine maximum for bar charts
    int highest = 1;
    for (auto val : devoMap) {
        highest = std::max(highest, val);
    }

    // Convert to QMap<QString,int> for ColorBar / ColorPie
    QMap<QString, int> mapSorted;
    for (auto it = devoMap.begin(); it != devoMap.end(); ++it) {
        mapSorted.insert(QString(it.key()), it.value());
    }

    // Color map
    QHash<QChar, QColor> colors = {{'W', QColor(248, 231, 185)}, {'U', QColor(14, 104, 171)},
                                   {'B', QColor(21, 11, 0)},     {'R', QColor(211, 32, 42)},
                                   {'G', QColor(0, 115, 62)},    {'C', QColor(150, 150, 150)}};

    // Choose display mode
    if (config.displayType == "bar") {
        // One BarWidget per devotion color
        for (auto c : devoMap.keys()) {
            QString label = QString("%1 %2 (%3)").arg(c).arg(devoMap[c]).arg(cardCount.value(QString(c)));

            BarWidget *bar = new BarWidget(label, devoMap[c], highest, colors.value(c, Qt::gray), this);

            barLayout->addWidget(bar);
        }
    } else if (config.displayType == "combinedBar") {
        // Stacked devotion bar
        ColorBar *cb = new ColorBar(mapSorted, this);
        cb->setMinimumHeight(30);
        barLayout->addWidget(cb);
    } else if (config.displayType == "pie") {
        // Devotion pie chart
        ColorPie *pie = new ColorPie(mapSorted, this);
        pie->setMinimumSize(200, 200);
        barLayout->addWidget(pie);
    }

    update();
}

QDialog *ManaDevotionWidget::createConfigDialog(QWidget *parent)
{
    ManaDevotionConfigDialog *dlg = new ManaDevotionConfigDialog(analyzer, config, parent);
    return dlg;
}

QJsonObject ManaDevotionWidget::extractConfigFromDialog(QDialog *dlg) const
{
    auto *mc = qobject_cast<ManaDevotionConfigDialog *>(dlg);
    if (!mc) {
        return {};
    }
    return mc->result().toJson();
}

QSize ManaDevotionWidget::sizeHint() const
{
    return QSize(800, 150);
}
