/**
 * @file mana_curve_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_CURVE_WIDGET_H
#define MANA_CURVE_WIDGET_H

#include "../../abstract_analytics_panel_widget.h"
#include "mana_curve_category_widget.h"
#include "mana_curve_config.h"
#include "mana_curve_total_widget.h"

#include <QVBoxLayout>

class SegmentedBarWidget;
class DeckListStatisticsAnalyzer;

class ManaCurveWidget : public AbstractAnalyticsPanelWidget
{
    Q_OBJECT

public slots:
    // QSize sizeHint() const override;
    void updateDisplay() override;
    QDialog *createConfigDialog(QWidget *parent) override;

public:
    ManaCurveWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer, ManaCurveConfig cfg = {});

    QJsonObject saveConfig() const override
    {
        return config.toJson();
    }
    void loadConfig(const QJsonObject &o) override
    {
        config = ManaCurveConfig::fromJson(o);
        updateDisplay();
    };

    QJsonObject extractConfigFromDialog(QDialog *dlg) const override;

private:
    ManaCurveConfig config;
    ManaCurveTotalWidget *totalWidget;
    ManaCurveCategoryWidget *categoryWidget;
};

#endif // MANA_CURVE_WIDGET_H
