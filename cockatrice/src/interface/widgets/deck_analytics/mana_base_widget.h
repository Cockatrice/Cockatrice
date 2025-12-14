/**
 * @file mana_base_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_BASE_WIDGET_H
#define MANA_BASE_WIDGET_H

#include "../general/display/banner_widget.h"
#include "deck_list_statistics_analyzer.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <utility>

class ManaBaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaBaseWidget(QWidget *parent, DeckListStatisticsAnalyzer *deckStatAnalyzer);
    void updateDisplay();

public slots:
    void retranslateUi();

private:
    DeckListStatisticsAnalyzer *deckStatAnalyzer;
    BannerWidget *bannerWidget;
    QVBoxLayout *layout;
    QWidget *barContainer;
    QHBoxLayout *barLayout;
};

#endif // MANA_BASE_WIDGET_H
