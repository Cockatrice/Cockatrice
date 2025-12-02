/**
 * @file mana_curve_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_CURVE_WIDGET_H
#define MANA_CURVE_WIDGET_H

#include "../general/display/banner_widget.h"
#include "deck_list_statistics_analyzer.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <unordered_map>

class ManaCurveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaCurveWidget(QWidget *parent, DeckListStatisticsAnalyzer *deckStatAnalyzer);
    void updateDisplay();

public slots:
    void retranslateUi();

private:
    DeckListStatisticsAnalyzer *deckStatAnalyzer;
    QVBoxLayout *layout;
    BannerWidget *bannerWidget;
    QWidget *barContainer;
    QHBoxLayout *barLayout;
};

#endif // MANA_CURVE_WIDGET_H
