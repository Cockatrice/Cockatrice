/**
 * @file mana_devotion_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 */
//! \todo Document this file.

#ifndef MANA_DEVOTION_WIDGET_H
#define MANA_DEVOTION_WIDGET_H
#include "../../abstract_analytics_panel_widget.h"
#include "mana_devotion_config.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <qtmetamacros.h>

class DeckListStatisticsAnalyzer;
class QHBoxLayout;
class QWidget;

class ManaDevotionWidget : public AbstractAnalyticsPanelWidget
{
    Q_OBJECT

public slots:
    QSize sizeHint() const override;
    void updateDisplay() override;
    QDialog *createConfigDialog(QWidget *parent) override;

public:
    ManaDevotionWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer, ManaDevotionConfig cfg = {});

    QJsonObject saveConfig() const override
    {
        return config.toJson();
    }
    void loadConfig(const QJsonObject &o) override
    {
        config = ManaDevotionConfig::fromJson(o);
        updateDisplay();
    }

    QJsonObject extractConfigFromDialog(QDialog *dlg) const override;

private:
    ManaDevotionConfig config;
    QWidget *barContainer;
    QHBoxLayout *barLayout;
};

#endif // MANA_DEVOTION_WIDGET_H
