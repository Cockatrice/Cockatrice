#include "mana_curve_total_widget.h"

#include "../../../general/display/charts/bars/bar_chart_widget.h"
#include "libcockatrice/utility/color.h"
#include "libcockatrice/utility/qt_utils.h"
#include "mana_curve_config.h"

#include <QHBoxLayout>

ManaCurveTotalWidget::ManaCurveTotalWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);

    label = new QLabel(this);
    label->setFixedWidth(80);
    layout->addWidget(label);

    barChart = new BarChartWidget(this);
    layout->addWidget(barChart, 1);

    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize ManaCurveTotalWidget::sizeHint() const
{
    return {0, 280};
}

QSize ManaCurveTotalWidget::minimumSizeHint() const
{
    return {0, 200};
}

void ManaCurveTotalWidget::updateDisplay(const QString &categoryName,
                                         int minCmc,
                                         int maxCmc,
                                         int highest,
                                         const QMap<int, QMap<QString, int>> &cmcMap,
                                         const QMap<QString, QMap<int, QStringList>> &cardsMap,
                                         const ManaCurveConfig &config)
{
    QVector<BarData> mainBars;
    mainBars.reserve(maxCmc - minCmc + 1);

    for (int cmc = minCmc; cmc <= maxCmc; ++cmc) {
        QVector<BarSegment> segments;

        const auto cmcIt = cmcMap.constFind(cmc);
        if (cmcIt != cmcMap.cend()) {
            for (auto it = cmcIt->cbegin(); it != cmcIt->cend(); ++it) {
                const QString &category = it.key();

                if (!config.filters.isEmpty() && !config.filters.contains(category))
                    continue;

                const int value = it.value();

                QStringList cards;
                const auto catIt = cardsMap.constFind(category);
                if (catIt != cardsMap.cend())
                    cards = catIt->value(cmc);

                segments.push_back({category, value, cards, GameSpecificColors::MTG::colorHelper(category)});
            }
        }

        std::sort(segments.begin(), segments.end(),
                  [](const BarSegment &a, const BarSegment &b) { return a.category < b.category; });

        mainBars.push_back({QString::number(cmc), segments});
    }

    label->setText(categoryName);

    barChart->setHighest(highest);
    barChart->setBars(mainBars);
}