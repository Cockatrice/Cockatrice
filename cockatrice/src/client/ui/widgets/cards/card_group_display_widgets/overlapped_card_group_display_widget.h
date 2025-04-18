#ifndef OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
#define OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H

#include "../../general/layout_containers/overlap_widget.h"
#include "card_group_display_widget.h"

class OverlappedCardGroupDisplayWidget : public CardGroupDisplayWidget
{
    Q_OBJECT

public:
    OverlappedCardGroupDisplayWidget(QWidget *parent,
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
    OverlapWidget *overlapWidget;
};

#endif // OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
