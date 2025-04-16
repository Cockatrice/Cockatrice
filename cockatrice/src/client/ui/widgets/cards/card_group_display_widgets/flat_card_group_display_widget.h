#ifndef FLAT_CARD_GROUP_DISPLAY_WIDGET_H
#define FLAT_CARD_GROUP_DISPLAY_WIDGET_H

#include "../../general/layout_containers/flow_widget.h"
#include "card_group_display_widget.h"

class FlatCardGroupDisplayWidget : public CardGroupDisplayWidget
{
    Q_OBJECT

public:
    FlatCardGroupDisplayWidget(QWidget *parent,
                               DeckListModel *deckListModel,
                               QString zoneName,
                               QString cardGroupCategory,
                               QString activeGroupCriteria,
                               QStringList activeSortCriteria,
                               int bannerOpacity,
                               CardSizeWidget *cardSizeWidget);

    void resizeEvent(QResizeEvent *event) override;

public slots:
    void updateCardDisplays() override;

private:
    FlowWidget *flowWidget;
};

#endif // FLAT_CARD_GROUP_DISPLAY_WIDGET_H
