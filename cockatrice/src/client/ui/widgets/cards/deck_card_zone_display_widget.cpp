#include "deck_card_zone_display_widget.h"

#include "overlapped_card_group_display_widget.h"

#include <QResizeEvent>

DeckCardZoneDisplayWidget::DeckCardZoneDisplayWidget(QWidget *parent,
                                                     QList<QPair<QString, QList<CardInfoPtr>>> cardLists,
                                                     QString bannerText,
                                                     int bannerOpacity,
                                                     int subBannerOpacity)
    : QWidget(parent), cardLists(cardLists), bannerOpacity(bannerOpacity), subBannerOpacity(subBannerOpacity)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    banner = new BannerWidget(this, bannerText, Qt::Orientation::Vertical, bannerOpacity);
    layout->addWidget(banner);

    displayCards();
}
void DeckCardZoneDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    for (QObject *child : layout->children()) {
        QWidget *widget = qobject_cast<QWidget *>(child);
        if (widget) {
            widget->setMaximumWidth(width());
        }
    }
}
void DeckCardZoneDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card);
}
void DeckCardZoneDisplayWidget::onHover(CardInfoPtr card)
{
    emit cardHovered(card);
}

void DeckCardZoneDisplayWidget::displayCards()
{
    for (QPair<QString, QList<CardInfoPtr>> &pair : cardLists) {
        QString criteriaValue = pair.first;
        QList<CardInfoPtr> cardList = pair.second;

        OverlappedCardGroupDisplayWidget *display_widget =
            new OverlappedCardGroupDisplayWidget(this, &cardList, criteriaValue, subBannerOpacity);
        connect(display_widget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
                SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
        connect(display_widget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
        layout->addWidget(display_widget);
    }
}