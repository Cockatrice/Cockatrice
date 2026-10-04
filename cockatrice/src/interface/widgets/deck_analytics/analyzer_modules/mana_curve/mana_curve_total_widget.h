#ifndef COCKATRICE_MANA_CURVE_TOTAL_WIDGET_H
#define COCKATRICE_MANA_CURVE_TOTAL_WIDGET_H
#include <QString>
#include <QStringList>
#include <QWidget>
#include <qtmetamacros.h>

class BarChartWidget;
class QHBoxLayout;
class QLabel;
struct ManaCurveConfig;
template <class Key, class T> class QMap;

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
