#ifndef COCKATRICE_MANA_DISTRIBUTION_SINGLE_DISPLAY_WIDGET_H
#define COCKATRICE_MANA_DISTRIBUTION_SINGLE_DISPLAY_WIDGET_H

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QWidget>

class ManaDistributionSingleDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ManaDistributionSingleDisplayWidget(const QString &colorSymbol, QWidget *parent = nullptr);

    void setDevotion(int pips, int cards, int percent);
    void setProduction(int pips, int cards, int percent);

private:
    QLabel *symbolLabel;

    QProgressBar *devotionBar;
    QLabel *devotionLabel;

    QProgressBar *productionBar;
    QLabel *productionLabel;
};

#endif // COCKATRICE_MANA_DISTRIBUTION_SINGLE_DISPLAY_WIDGET_H
