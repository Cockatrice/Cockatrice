#ifndef DECK_ANALYTICS_WIDGET_H
#define DECK_ANALYTICS_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../ui/widgets/general/layout_containers/flow_widget.h"
#include "mana_base_widget.h"
#include "mana_curve_widget.h"
#include "mana_devotion_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <decklist.h>
#include <qscrollarea.h>

class DeckAnalyticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckAnalyticsWidget(QWidget *parent = nullptr, DeckListModel *deck_list_model = nullptr);
    void setDeckList(const DeckList &new_deck_list_model);
    std::map<int, int> analyzeManaCurve();

private:
    DeckListModel *deck_list_model;
    QHBoxLayout *main_layout;
    FlowWidget *flow_widget;

    ManaCurveWidget *mana_curve_widget;
    ManaDevotionWidget *mana_devotion_widget;
    ManaBaseWidget *mana_base_widget;
};

#endif // DECK_ANALYTICS_WIDGET_H
