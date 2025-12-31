/**
 * @file mana_base_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_BASE_WIDGET_H
#define MANA_BASE_WIDGET_H

#include "../../../general/display/banner_widget.h"
#include "../../abstract_analytics_panel_widget.h"
#include "../../deck_list_statistics_analyzer.h"
#include "mana_base_config.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <utility>

class ManaBaseWidget : public AbstractAnalyticsPanelWidget
{
    Q_OBJECT

public slots:
    QSize sizeHint() const override;
    void updateDisplay() override;
    QDialog *createConfigDialog(QWidget *parent) override;

public:
    ManaBaseWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer, ManaBaseConfig cfg = {});

    QJsonObject saveConfig() const override
    {
        return config.toJson();
    }
    void loadConfig(const QJsonObject &o) override
    {
        config = ManaBaseConfig::fromJson(o);
        updateDisplay();
    }

    QJsonObject extractConfigFromDialog(QDialog *dlg) const override;

private:
    ManaBaseConfig config;
    QWidget *barContainer;
    QHBoxLayout *barLayout;
};

#endif // MANA_BASE_WIDGET_H
