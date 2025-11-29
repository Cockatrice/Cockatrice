/**
 * @file deck_analytics_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_ANALYTICS_WIDGET_H
#define DECK_ANALYTICS_WIDGET_H

#include "mana_base_widget.h"
#include "mana_curve_widget.h"
#include "mana_devotion_widget.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>

class DeckAnalyticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckAnalyticsWidget(QWidget *parent, DeckListModel *deckListModel);
    void setDeckList(const DeckList &_deckListModel);
    std::map<int, int> analyzeManaCurve();
    void refreshDisplays(DeckListModel *_deckListModel);

private:
    DeckListModel *deckListModel;
    QVBoxLayout *mainLayout;

    QWidget *container;
    QVBoxLayout *containerLayout;

    QScrollArea *scrollArea;

    ManaCurveWidget *manaCurveWidget;
    ManaDevotionWidget *manaDevotionWidget;
    ManaBaseWidget *manaBaseWidget;
};

#endif // DECK_ANALYTICS_WIDGET_H
