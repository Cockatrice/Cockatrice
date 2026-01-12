#include "mana_distribution_widget.h"

#include "../../analytics_panel_widget_registrar.h"
#include "mana_distribution_config_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

namespace
{
AnalyticsPanelWidgetRegistrar registerManaDistribution{
    "manaProdDevotion", ManaDistributionWidget::tr("Mana Production + Devotion"),
    [](QWidget *parent, DeckListStatisticsAnalyzer *analyzer) { return new ManaDistributionWidget(parent, analyzer); }};

} // anonymous namespace

static const QStringList kColors = {"W", "U", "B", "R", "G", "C"};

ManaDistributionWidget::ManaDistributionWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer)
    : AbstractAnalyticsPanelWidget(parent, analyzer)
{
    container = new QWidget(this);
    containerLayout = new QVBoxLayout(container);

    devotionBarTop = new ColorBar({}, this);
    devotionPieTop = new ColorPie({}, this);
    productionBarTop = new ColorBar({}, this);
    productionPieTop = new ColorPie({}, this);

    containerLayout->addWidget(devotionBarTop);
    containerLayout->addWidget(devotionPieTop);
    containerLayout->addWidget(productionBarTop);
    containerLayout->addWidget(productionPieTop);

    devotionPieTop->hide();
    productionPieTop->hide();

    row = new QHBoxLayout();
    containerLayout->addLayout(row);

    for (const QString &c : kColors) {
        auto *w = new ManaDistributionSingleDisplayWidget(c, this);
        row->addWidget(w);
        rows[c] = w;
    }

    layout->addWidget(container);
}

void ManaDistributionWidget::updateDisplay()
{
    const auto &devPips = analyzer->getDevotionPipCount();
    const auto &devCards = analyzer->getDevotionCardCount();
    const auto &prodPips = analyzer->getProductionPipCount();
    const auto &prodCards = analyzer->getProductionCardCount();

    QStringList filtered = config.filters.isEmpty() ? kColors : config.filters;

    QMap<QString, int> devMap, prodMap;
    for (const QString &c : filtered) {
        devMap[c] = devPips.value(c, 0);
        prodMap[c] = prodPips.value(c, 0);
    }

    bool showPie = (config.displayType == "pie");

    devotionBarTop->setVisible(!showPie);
    productionBarTop->setVisible(!showPie);

    devotionPieTop->setVisible(showPie);
    productionPieTop->setVisible(showPie);

    if (showPie) {
        devotionPieTop->setColors(devMap);
        productionPieTop->setColors(prodMap);
    } else {
        devotionBarTop->setColors(devMap);
        productionBarTop->setColors(prodMap);
    }

    for (const QString &c : kColors) {
        auto *w = rows.value(c);

        if (!w) {
            continue;
        }

        bool visible = config.showColorRows && filtered.contains(c);
        w->setVisible(visible);
        if (!visible) {
            continue;
        }

        int dp = devPips.value(c, 0);
        int dc = devCards.value(c, 0);
        int pp = prodPips.value(c, 0);
        int pc = prodCards.value(c, 0);

        // Compute percentages
        int totalDev = 0;
        int totalProd = 0;
        for (const QString &cc : filtered) {
            totalDev += devPips.value(cc, 0);
            totalProd += prodPips.value(cc, 0);
        }

        int devPct = (totalDev > 0) ? int(100.0 * dp / totalDev) : 0;
        int prodPct = (totalProd > 0) ? int(100.0 * pp / totalProd) : 0;

        w->setDevotion(dp, dc, devPct);
        w->setProduction(pp, pc, prodPct);
    }
}

QDialog *ManaDistributionWidget::createConfigDialog(QWidget *parent)
{
    auto *dlg = new ManaDistributionConfigDialog(analyzer, parent);
    dlg->setWindowTitle(tr("Mana Distribution Settings"));
    dlg->setFromConfig(config);

    connect(dlg, &QDialog::accepted, [this, dlg]() {
        config = dlg->config();
        updateDisplay();
    });

    return dlg;
}
