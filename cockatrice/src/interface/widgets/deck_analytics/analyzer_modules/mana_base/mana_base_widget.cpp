#include "mana_base_widget.h"

#include "../../../general/display/charts/bars/bar_widget.h"
#include "../../../general/display/charts/bars/color_bar.h"
#include "../../../general/display/charts/pies/color_pie.h"
#include "../../analytics_panel_widget_registrar.h"
#include "mana_base_config_dialog.h"

#include <QDialog>
#include <QListWidget>

namespace
{

AnalyticsPanelWidgetRegistrar registerManaBase{
    "manaBase", ManaBaseWidget::tr("Mana Base"),
    [](QWidget *parent, DeckListStatisticsAnalyzer *analyzer) { return new ManaBaseWidget(parent, analyzer); }};

} // anonymous namespace

ManaBaseWidget::ManaBaseWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer, ManaBaseConfig cfg)
    : AbstractAnalyticsPanelWidget(parent, analyzer), config(std::move(cfg))
{
    barContainer = new QWidget(this);
    barLayout = new QHBoxLayout(barContainer);
    layout->addWidget(barContainer);

    updateDisplay();
}

void ManaBaseWidget::updateDisplay()
{
    // Clear previous widgets
    while (QLayoutItem *item = barLayout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    auto &pipCount = analyzer->getProductionPipCount();
    auto &cardCount = analyzer->getProductionCardCount();

    QHash<QString, int> manaMap;
    for (auto key : pipCount.keys()) {
        manaMap[key] = pipCount[key];
    }

    // Apply filters
    if (!config.filters.isEmpty()) {
        QHash<QString, int> filtered;
        for (auto f : config.filters) {
            if (manaMap.contains(f)) {
                filtered[f] = manaMap[f];
            }
        }
        manaMap = filtered;
    }

    // Determine maximum for bar charts
    int highest = 1;
    for (auto val : manaMap) {
        highest = std::max(highest, val);
    }

    // Convert to QMap for ColorBar / ColorPie (sorted)
    QMap<QString, int> mapSorted;
    for (auto it = manaMap.begin(); it != manaMap.end(); ++it) {
        mapSorted.insert(it.key(), it.value());
    }

    // Choose display mode
    if (config.displayType == "bar") {
        QHash<QString, QColor> colors = {{"W", QColor(248, 231, 185)}, {"U", QColor(14, 104, 171)},
                                         {"B", QColor(21, 11, 0)},     {"R", QColor(211, 32, 42)},
                                         {"G", QColor(0, 115, 62)},    {"C", QColor(150, 150, 150)}};

        for (auto color : manaMap.keys()) {
            QString label = QString("%1 %2 (%3)").arg(color).arg(manaMap[color]).arg(cardCount.value(color));

            BarWidget *bar = new BarWidget(label, manaMap[color], highest, colors.value(color, Qt::gray), this);

            barLayout->addWidget(bar);
        }
    } else if (config.displayType == "combinedBar") {
        ColorBar *cb = new ColorBar(mapSorted, this);
        cb->setMinimumHeight(30);
        barLayout->addWidget(cb);
    } else if (config.displayType == "pie") {
        ColorPie *pie = new ColorPie(mapSorted, this);
        pie->setMinimumSize(200, 200);
        barLayout->addWidget(pie);
    }

    update();
}
QSize ManaBaseWidget::sizeHint() const
{
    return QSize(800, 150);
}

QDialog *ManaBaseWidget::createConfigDialog(QWidget *parent)
{
    ManaBaseConfigDialog *dlg = new ManaBaseConfigDialog(analyzer, config, parent);
    return dlg;
}

QJsonObject ManaBaseWidget::extractConfigFromDialog(QDialog *dlg) const
{
    auto *mc = qobject_cast<ManaBaseConfigDialog *>(dlg);
    if (!mc) {
        return {};
    }
    return mc->result().toJson();
}