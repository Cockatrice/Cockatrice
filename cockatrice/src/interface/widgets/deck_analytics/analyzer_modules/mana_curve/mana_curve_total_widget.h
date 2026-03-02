#ifndef COCKATRICE_MANA_CURVE_TOTAL_WIDGET_H
#define COCKATRICE_MANA_CURVE_TOTAL_WIDGET_H
#include "../../../general/display/charts/bars/bar_chart_widget.h"
#include "mana_curve_config.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class ManaCurveTotalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaCurveTotalWidget(QWidget *parent);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void updateDisplay(const QString &categoryName,
                       int minCmc,
                       int maxCmc,
                       int highest,
                       const QMap<int, QMap<QString, int>> &cmcMap,
                       const QMap<QString, QMap<int, QStringList>> &cardsMap,
                       const ManaCurveConfig &config);

private:
    QHBoxLayout *layout;
    QLabel *label;
    BarChartWidget *barChart;
};

#endif // COCKATRICE_MANA_CURVE_TOTAL_WIDGET_H
