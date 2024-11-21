#include "overlapped_card_group_display_widget.h"

#include "card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>

OverlappedCardGroupDisplayWidget::OverlappedCardGroupDisplayWidget(QWidget *parent,
                                                                   QList<CardInfoPtr> *cards,
                                                                   QString bannerText,
                                                                   int bannerOpacity)
    : QWidget(parent), cardsToDisplay(cards)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setMinimumSize(QSize(0, 0));

    banner = new BannerWidget(bannerText, Qt::Orientation::Vertical, bannerOpacity, this);
    overlapWidget = new OverlapWidget(this, 80, 1, 1, Qt::Vertical, true);

    layout->addWidget(banner);
    layout->addWidget(overlapWidget);
    addCardsToOverlapWidget();
}

void OverlappedCardGroupDisplayWidget::addCardsToOverlapWidget()
{
    for (int i = 0; i < cardsToDisplay->size(); i++) {
        CardInfoPtr card = cardsToDisplay->at(i);
        if (card) {
            CardInfoPictureWithTextOverlayWidget *display =
                new CardInfoPictureWithTextOverlayWidget(overlapWidget, true);
            display->setCard(card);

            connect(display, SIGNAL(imageClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
                    SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
            connect(display, SIGNAL(hoveredOnCard(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
            overlapWidget->addWidget(display);
        }
    }
    overlapWidget->adjustMaxColumnsAndRows();
}

void OverlappedCardGroupDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card);
}

void OverlappedCardGroupDisplayWidget::onHover(CardInfoPtr card)
{
    emit cardHovered(card);
}

void OverlappedCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    overlapWidget->resize(event->size());
    overlapWidget->adjustMaxColumnsAndRows();
}