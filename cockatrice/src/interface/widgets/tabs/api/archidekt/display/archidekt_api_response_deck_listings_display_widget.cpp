#include "archidekt_api_response_deck_listings_display_widget.h"

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "archidekt_api_response_deck_entry_display_widget.h"

ArchidektApiResponseDeckListingsDisplayWidget::ArchidektApiResponseDeckListingsDisplayWidget(
    QWidget *parent,
    ArchidektDeckListingApiResponse response,
    CardSizeWidget *_cardSizeSlider)
    : QWidget(parent), cardSizeSlider(_cardSizeSlider)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    imageNetworkManager = new QNetworkAccessManager(this);
    imageNetworkManager->setTransferTimeout(); // Use Qt's default timeout
    imageNetworkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    // Add widgets for deck listings
    auto deckListings = response.results;
    for (const auto &deckListing : deckListings) {
        auto cardListDisplayWidget =
            new ArchidektApiResponseDeckEntryDisplayWidget(this, deckListing, imageNetworkManager);
        cardListDisplayWidget->setScaleFactor(cardSizeSlider->getSlider()->value());
        connect(cardListDisplayWidget, &ArchidektApiResponseDeckEntryDisplayWidget::requestNavigation, this,
                &ArchidektApiResponseDeckListingsDisplayWidget::requestNavigation);
        connect(cardSizeSlider->getSlider(), &QSlider::valueChanged, cardListDisplayWidget,
                &ArchidektApiResponseDeckEntryDisplayWidget::setScaleFactor);
        flowWidget->addWidget(cardListDisplayWidget);
    }

    layout->addWidget(flowWidget);
}

void ArchidektApiResponseDeckListingsDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layout->invalidate();
    layout->activate();
    layout->update();
}
