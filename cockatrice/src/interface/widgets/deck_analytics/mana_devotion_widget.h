/**
 * @file mana_devotion_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_DEVOTION_WIDGET_H
#define MANA_DEVOTION_WIDGET_H

#include "../general/display/banner_widget.h"
#include "deck_list_statistics_analyzer.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <utility>

class ManaDevotionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaDevotionWidget(QWidget *parent, DeckListStatisticsAnalyzer *deckStatAnalyzer);
    void updateDisplay();

public slots:
    void retranslateUi();

private:
    DeckListStatisticsAnalyzer *deckStatAnalyzer;
    BannerWidget *bannerWidget;
    QVBoxLayout *layout;
    QHBoxLayout *barLayout;
};

#endif // MANA_DEVOTION_WIDGET_H
