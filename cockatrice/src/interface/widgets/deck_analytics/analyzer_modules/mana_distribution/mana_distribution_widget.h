#ifndef COCKATRICE_MANA_DISTRIBUTION_WIDGET_H
#define COCKATRICE_MANA_DISTRIBUTION_WIDGET_H

#include "../../abstract_analytics_panel_widget.h"
#include "mana_distribution_config.h"

#include <QJsonObject>
#include <QMap>
#include <QString>
#include <qtmetamacros.h>

class ColorBar;
class ColorPie;
class DeckListStatisticsAnalyzer;
class ManaDistributionSingleDisplayWidget;
class QHBoxLayout;
class QVBoxLayout;
class QWidget;

class ManaDistributionWidget : public AbstractAnalyticsPanelWidget
{
    Q_OBJECT
public:
    explicit ManaDistributionWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer);

    void updateDisplay() override;
    QDialog *createConfigDialog(QWidget *parent) override;
    QJsonObject extractConfigFromDialog(QDialog *) const override
    {
        return {};
    }

private:
    ManaDistributionConfig config;

    QWidget *container;
    QVBoxLayout *containerLayout;

    QVBoxLayout *topLayout;
    ColorBar *devotionBarTop;
    ColorPie *devotionPieTop;
    ColorBar *productionBarTop;
    ColorPie *productionPieTop;

    QHBoxLayout *row;
    QMap<QString, ManaDistributionSingleDisplayWidget *> rows;
};

#endif // COCKATRICE_MANA_DISTRIBUTION_WIDGET_H
