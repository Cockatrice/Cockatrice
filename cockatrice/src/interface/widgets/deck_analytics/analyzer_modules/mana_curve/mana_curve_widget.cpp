#include "mana_curve_widget.h"

#include "../../../general/display/charts/bars/bar_chart_background_widget.h"
#include "../../../general/display/charts/bars/bar_chart_widget.h"
#include "../../../general/display/charts/bars/segmented_bar_widget.h"
#include "../../analytics_panel_widget_registrar.h"
#include "../../deck_list_statistics_analyzer.h"
#include "libcockatrice/utility/color.h"
#include "libcockatrice/utility/qt_utils.h"
#include "mana_curve_config_dialog.h"

#include <QInputDialog>
#include <QJsonArray>
#include <QLabel>
#include <QPushButton>
#include <QSettings>

namespace
{

AnalyticsPanelWidgetRegistrar registerManaCurve{
    "manaCurve", ManaCurveWidget::tr("Mana Curve"),
    [](QWidget *parent, DeckListStatisticsAnalyzer *analyzer) { return new ManaCurveWidget(parent, analyzer); }};

} // anonymous namespace

ManaCurveWidget::ManaCurveWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer, ManaCurveConfig cfg)
    : AbstractAnalyticsPanelWidget(parent, analyzer), config(cfg)
{
    setLayout(layout);

    totalWidget = new ManaCurveTotalWidget(this);
    totalWidget->setHidden(true);
    layout->addWidget(totalWidget);

    categoryWidget = new ManaCurveCategoryWidget(this);
    categoryWidget->setHidden(true);
    layout->addWidget(categoryWidget);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(analyzer, &DeckListStatisticsAnalyzer::statsUpdated, this, &ManaCurveWidget::updateDisplay);

    updateDisplay();
}

QDialog *ManaCurveWidget::createConfigDialog(QWidget *parent)
{
    auto *dlg = new ManaCurveConfigDialog(analyzer, parent);
    dlg->setFromConfig(config);
    return dlg;
}

QJsonObject ManaCurveWidget::extractConfigFromDialog(QDialog *dlg) const
{
    auto *mc = qobject_cast<ManaCurveConfigDialog *>(dlg);
    return mc ? mc->result().toJson() : QJsonObject{};
}

static void buildMapsByCategory(const QHash<QString, QHash<int, int>> &categoryCounts,
                                const QHash<QString, QHash<int, QStringList>> &categoryCards,
                                QMap<int, QMap<QString, int>> &outCmcMap,
                                QMap<QString, QMap<int, QStringList>> &outCardsMap)
{
    outCmcMap.clear();
    outCardsMap.clear();

    for (auto catIt = categoryCounts.cbegin(); catIt != categoryCounts.cend(); ++catIt) {
        const QString &category = catIt.key();
        const auto &countsByCmc = catIt.value();

        for (auto it = countsByCmc.cbegin(); it != countsByCmc.cend(); ++it)
            outCmcMap[it.key()][category] = it.value();
    }

    for (auto catIt = categoryCards.cbegin(); catIt != categoryCards.cend(); ++catIt) {
        const QString &category = catIt.key();
        const auto &cardsByCmc = catIt.value();

        for (auto it = cardsByCmc.cbegin(); it != cardsByCmc.cend(); ++it)
            outCardsMap[category][it.key()] = it.value();
    }
}

static void findGlobalCmcRange(const QHash<QString, QHash<int, int>> &categoryCounts, int &minCmc, int &maxCmc)
{
    minCmc = 0;
    maxCmc = 0;

    for (const auto &countsByCmc : categoryCounts) {
        for (auto it = countsByCmc.cbegin(); it != countsByCmc.cend(); ++it)
            maxCmc = qMax(maxCmc, it.key());
    }
}

void ManaCurveWidget::updateDisplay()
{
    QHash<QString, QHash<int, int>> categoryCounts;
    QHash<QString, QHash<int, QStringList>> categoryCards;

    if (config.groupBy == "color") {
        categoryCounts = analyzer->getManaCurveByColor();
        categoryCards = analyzer->getManaCurveCardsByColor();
    } else if (config.groupBy == "subtype") {
        categoryCounts = analyzer->getManaCurveBySubtype();
        categoryCards = analyzer->getManaCurveCardsBySubtype();
    } else if (config.groupBy == "power") {
        categoryCounts = analyzer->getManaCurveByPower();
        categoryCards = analyzer->getManaCurveCardsByPower();
    } else {
        categoryCounts = analyzer->getManaCurveByType();
        categoryCards = analyzer->getManaCurveCardsByType();
    }

    QMap<int, QMap<QString, int>> cmcMap;
    QMap<QString, QMap<int, QStringList>> cardsMap;
    buildMapsByCategory(categoryCounts, categoryCards, cmcMap, cardsMap);

    int minCmc = 0;
    int maxCmc = 0;
    findGlobalCmcRange(categoryCounts, minCmc, maxCmc);

    int highest = 1;
    for (int cmc = minCmc; cmc <= maxCmc; ++cmc) {
        int sum = 0;

        const auto cmcIt = cmcMap.constFind(cmc);
        if (cmcIt != cmcMap.cend()) {
            for (auto it = cmcIt->cbegin(); it != cmcIt->cend(); ++it) {
                if (!config.filters.isEmpty() && !config.filters.contains(it.key())) {
                    continue;
                }

                sum += it.value();
            }
        }

        highest = qMax(highest, sum);
    }

    totalWidget->updateDisplay(config.groupBy, minCmc, maxCmc, highest, cmcMap, cardsMap, config);

    totalWidget->setVisible(config.showMain);

    categoryWidget->updateDisplay(minCmc, maxCmc, highest, categoryCounts, categoryCards, config);

    categoryWidget->setVisible(config.showCategoryRows);
}
