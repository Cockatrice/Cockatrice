/**
 * @file mana_base_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 */
//! \todo Document this file.

#ifndef MANA_BASE_WIDGET_H
#define MANA_BASE_WIDGET_H

#include "../../abstract_analytics_panel_widget.h"
#include "mana_base_config.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <qtmetamacros.h>

class DeckListStatisticsAnalyzer;
class QHBoxLayout;
class QWidget;

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
