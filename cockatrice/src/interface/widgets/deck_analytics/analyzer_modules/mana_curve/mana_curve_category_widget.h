#ifndef COCKATRICE_MANA_CURVE_CATEGORY_WIDGET_H
#define COCKATRICE_MANA_CURVE_CATEGORY_WIDGET_H

#include "../../../general/display/charts/bars/bar_chart_widget.h"
#include "mana_curve_config.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class ManaCurveCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaCurveCategoryWidget(QWidget *parent);
    void updateDisplay(int minCmc,
                       int maxCmc,
                       int highest,
                       QHash<QString, QHash<int, int>> qCategoryCounts,
                       QHash<QString, QHash<int, QStringList>> qCategoryCards,
                       const ManaCurveConfig &config);

public slots:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QVBoxLayout *layout;
};

#endif // COCKATRICE_MANA_CURVE_CATEGORY_WIDGET_H
