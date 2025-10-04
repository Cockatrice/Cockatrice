/**
 * @file deck_analytics_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_ANALYTICS_WIDGET_H
#define DECK_ANALYTICS_WIDGET_H

#include "../../../deck/deck_list_model.h"
#include "../general/layout_containers/flow_widget.h"
#include "mana_base_widget.h"
#include "mana_curve_widget.h"
#include "mana_devotion_widget.h"

#include <../../../../../libs/deck/deck_list.h>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

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
