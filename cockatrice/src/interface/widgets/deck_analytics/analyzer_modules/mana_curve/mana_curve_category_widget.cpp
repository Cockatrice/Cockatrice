#include "mana_curve_category_widget.h"

#include "libcockatrice/utility/color.h"
#include "libcockatrice/utility/qt_utils.h"
#include "mana_curve_config.h"
#include "mana_curve_total_widget.h"

constexpr int MIN_ROW_HEIGHT = 100; // Minimum readable height per row

ManaCurveCategoryWidget::ManaCurveCategoryWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(0, 0, 0, 0);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

// Same as minimum for now
QSize ManaCurveCategoryWidget::sizeHint() const
{
    if (layout->isEmpty()) {
        return QSize(0, 0);
    }

    // Calculate exact height needed for all rows
    int rowCount = layout->count();

    int totalHeight = rowCount * MIN_ROW_HEIGHT;
    totalHeight += (rowCount - 1) * layout->spacing();

    return QSize(0, totalHeight);
}

QSize ManaCurveCategoryWidget::minimumSizeHint() const
{
    if (layout->isEmpty()) {
        return QSize(0, 0);
    }

    // Calculate actual minimum based on number of rows
    int rowCount = layout->count();

    int totalHeight = rowCount * MIN_ROW_HEIGHT;
    totalHeight += (rowCount - 1) * layout->spacing();

    return QSize(0, totalHeight);
}

void ManaCurveCategoryWidget::updateDisplay(int minCmc,
                                            int maxCmc,
                                            int highest,
                                            QHash<QString, QHash<int, int>> qCategoryCounts,
                                            QHash<QString, QHash<int, QStringList>> qCategoryCards,
                                            const ManaCurveConfig &config)
{
    // Clear previous content
    QtUtils::clearLayoutRec(layout);

    if (!config.showCategoryRows) {
        return; // nothing to show
    }

    // Collect categories
    QStringList categories = qCategoryCounts.keys();

    // Apply filters
    if (!config.filters.isEmpty()) {
        QStringList filtered;
        for (const QString &cat : categories) {
            if (config.filters.contains(cat)) {
                filtered.append(cat);
            }
        }
        categories = filtered;
    }

    std::sort(categories.begin(), categories.end());

    for (const QString &cat : categories) {
        QWidget *row = new QWidget(this);
        row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        row->setFixedHeight(MIN_ROW_HEIGHT);

        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(4);

        QLabel *categoryLabel = new QLabel(cat, row);
        categoryLabel->setFixedWidth(80);
        rowLayout->addWidget(categoryLabel);

        QVector<BarData> catBars;
        const auto cmcCounts = qCategoryCounts.value(cat);
        const auto cmcCards = qCategoryCards.value(cat);

        for (int cmc = minCmc; cmc <= maxCmc; ++cmc) {
            int val = cmcCounts.value(cmc, 0);
            QStringList cards = cmcCards.value(cmc);

            QVector<BarSegment> segments;
            if (val > 0) {
                segments.push_back({cat, val, cards, GameSpecificColors::MTG::colorHelper(cat)});
            }

            catBars.push_back({QString::number(cmc), segments});
        }

        auto *catChart = new BarChartWidget(row);
        catChart->setHighest(highest);
        catChart->setBars(catBars);
        catChart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        rowLayout->addWidget(catChart);
        layout->addWidget(row);
    }

    // Update geometry after adding all widgets
    updateGeometry();
}