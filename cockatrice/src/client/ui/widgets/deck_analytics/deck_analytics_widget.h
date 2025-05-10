#ifndef DECK_ANALYTICS_WIDGET_H
#define DECK_ANALYTICS_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../ui/widgets/general/layout_containers/flow_widget.h"
#include "mana_base_widget.h"
#include "mana_curve_widget.h"
#include "mana_devotion_widget.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <decklist.h>

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
