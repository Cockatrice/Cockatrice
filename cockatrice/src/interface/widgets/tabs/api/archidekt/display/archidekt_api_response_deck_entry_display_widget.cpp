#include "archidekt_api_response_deck_entry_display_widget.h"

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"

ArchidektApiResponseDeckEntryDisplayWidget::ArchidektApiResponseDeckEntryDisplayWidget(
    QWidget *parent,
    ArchidektApiResponseDeckListingContainer _response)
    : QWidget(parent), response(_response)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    // Add widgets for deck listings

    deckPreviewDisplayWidget = new CardInfoPictureWithTextOverlayWidget(this);
    deckPreviewDisplayWidget->setOverlayText(QString("%1 (%2)").arg(response.getName()).arg(response.getSize()));
    connect(deckPreviewDisplayWidget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this,
            &ArchidektApiResponseDeckEntryDisplayWidget::actRequestNavigationToDeck);

    layout->addWidget(deckPreviewDisplayWidget);
}

void ArchidektApiResponseDeckEntryDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layout->invalidate();
    layout->activate();
    layout->update();
}

void ArchidektApiResponseDeckEntryDisplayWidget::actRequestNavigationToDeck()
{
    emit requestNavigation(QString("https://archidekt.com/api/decks/%1/").arg(response.getId()));
}
