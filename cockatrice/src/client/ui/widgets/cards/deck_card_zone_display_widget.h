#ifndef DECK_CARD_ZONE_DISPLAY_WIDGET_H
#define DECK_CARD_ZONE_DISPLAY_WIDGET_H

#include "../../../../game/cards/card_database.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/overlap_widget.h"
#include "card_info_picture_with_text_overlay_widget.h"

#include <QVBoxLayout>
#include <QWidget>

class DeckCardZoneDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    DeckCardZoneDisplayWidget(QWidget *parent,
                              QList<QPair<QString, QList<CardInfoPtr>>> cardLists,
                              QString bannerText,
                              int bannerOpacity,
                              int subBannerOpacity);
    void addCardsToOverlapWidget();
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(CardInfoPtr card);
    void displayCards();

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void cardHovered(CardInfoPtr card);

private:
    QList<QPair<QString, QList<CardInfoPtr>>> cardLists;
    QVBoxLayout *layout;
    BannerWidget *banner;
    int bannerOpacity = 20;
    int subBannerOpacity = 10;
    OverlapWidget *overlapWidget;
};

#endif // DECK_CARD_ZONE_DISPLAY_WIDGET_H
