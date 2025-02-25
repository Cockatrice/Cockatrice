//
// Created by ascor on 11/12/24.
//

#ifndef OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
#define OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H

#include "../../../../game/cards/card_database.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/overlap_widget.h"
#include "card_info_picture_with_text_overlay_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class OverlappedCardGroupDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    OverlappedCardGroupDisplayWidget(QWidget *parent, QList<CardInfoPtr> *cards, QString bannerText, int bannerOpacity);
    void addCardsToOverlapWidget();
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(CardInfoPtr card);

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void cardHovered(CardInfoPtr card);

private:
    QList<CardInfoPtr> *cardsToDisplay;
    QVBoxLayout *layout;
    BannerWidget *banner;
    OverlapWidget *overlapWidget;
};

#endif // OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
