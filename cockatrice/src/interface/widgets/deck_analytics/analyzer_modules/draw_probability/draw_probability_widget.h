#ifndef COCKATRICE_DRAW_PROBABILITY_WIDGET_H
#define COCKATRICE_DRAW_PROBABILITY_WIDGET_H

#include "../../abstract_analytics_panel_widget.h"
#include "draw_probability_config.h"

#include <qtmetamacros.h>

class DeckListStatisticsAnalyzer;
class FlowWidget;
class QComboBox;
class QLabel;
class QSpinBox;
class QTableWidget;
class QWidget;

class DrawProbabilityWidget : public AbstractAnalyticsPanelWidget
{
    Q_OBJECT
public:
    DrawProbabilityWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer);

    QDialog *createConfigDialog(QWidget *parent) override;
    QJsonObject extractConfigFromDialog(QDialog *dlg) const override;
    void applyConfigToToolbar();

public slots:
    void updateDisplay() override;
    void loadConfig(const QJsonObject &cfg) override;
    void retranslateUi();

private slots:
    void updateFilterOptions();

private:
    DrawProbabilityConfig config;

    FlowWidget *controls;
    QLabel *labelPrefix;
    QLabel *labelMiddle;
    QLabel *labelSuffix;
    QComboBox *criteriaCombo;  // Card Name / Type / Subtype / Mana Value
    QComboBox *exactnessCombo; // At least / Exactly
    QSpinBox *quantitySpin;    // N
    QSpinBox *drawnSpin;       // M

    QLabel *populationLabel;
    QTableWidget *resultTable;
};

#endif // COCKATRICE_DRAW_PROBABILITY_WIDGET_H
