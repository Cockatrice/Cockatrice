#include "archidekt_api_response_deck_listings_display_widget.h"

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "archidekt_api_response_deck_entry_display_widget.h"

ArchidektApiResponseDeckListingsDisplayWidget::ArchidektApiResponseDeckListingsDisplayWidget(
    QWidget *parent,
    ArchidektDeckListingApiResponse response)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    imageNetworkManager = new QNetworkAccessManager(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    imageNetworkManager->setTransferTimeout(); // Use Qt's default timeout
#endif

    imageNetworkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    // Add widgets for deck listings
    auto deckListings = response.results;
    for (const auto &deckListing : deckListings) {
        auto cardListDisplayWidget = new ArchidektApiResponseDeckEntryDisplayWidget(this, deckListing, imageNetworkManager);
        connect(cardListDisplayWidget, &ArchidektApiResponseDeckEntryDisplayWidget::requestNavigation, this,
                &ArchidektApiResponseDeckListingsDisplayWidget::requestNavigation);
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
