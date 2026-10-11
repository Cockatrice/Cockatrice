#ifndef COCKATRICE_DRAW_PROBABILITY_WIDGET_H
#define COCKATRICE_DRAW_PROBABILITY_WIDGET_H

#include "../../abstract_analytics_panel_widget.h"
#include "draw_probability_config.h"

#include <qtmetamacros.h>

class DeckListStatisticsAnalyzer;
class FlowWidget;
class QComboBox;
class QLabel;
class QLineEdit;
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
    QLineEdit *cardNameEdit;
    QComboBox *criteriaCombo;  // Card Name / Type / Subtype / Mana Value
    QComboBox *filterCombo;    // The actual value
    QComboBox *exactnessCombo; // At least / Exactly
    QSpinBox *quantitySpin;    // N
    QSpinBox *drawnSpin;       // M

    QSpinBox *manaValueSpin;

    QTableWidget *resultTable;

    double hypergeometricProbability(int N, int K, int n, int k);
    double calculateProbability(int totalCards, int copies, int drawn, bool atLeast);
};

#endif // COCKATRICE_DRAW_PROBABILITY_WIDGET_H
