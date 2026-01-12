#ifndef COCKATRICE_MANA_DISTRIBUTION_WIDGET_H
#define COCKATRICE_MANA_DISTRIBUTION_WIDGET_H

#include "../../../general/display/charts/bars/color_bar.h"
#include "../../../general/display/charts/pies/color_pie.h"
#include "../../abstract_analytics_panel_widget.h"
#include "../../deck_list_statistics_analyzer.h"
#include "mana_distribution_config.h"
#include "mana_distribution_single_display_widget.h"

#include <QHBoxLayout>
#include <QMap>
#include <QVBoxLayout>
#include <QWidget>

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
