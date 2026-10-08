#include "archidekt_api_response_deck_listings_display_widget.h"

#include "../../../../cards/card_size_widget.h"
#include "../../../../general/layout_containers/flow_widget.h"
#include "../api_response/archidekt_deck_listing_api_response.h"
#include "../api_response/deck_listings/archidekt_api_response_deck_listing_container.h"
#include "archidekt_api_response_deck_entry_display_widget.h"

#include <QHBoxLayout>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSlider>
#include <qnamespace.h>

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

    cardSizeSlider->enableCtrlScrollResize(flowWidget);

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

void ArchidektApiResponseDeckListingsDisplayWidget::append(const ArchidektDeckListingApiResponse &data)
{
    for (const auto &deckListing : data.results) {
        auto cardListDisplayWidget =
            new ArchidektApiResponseDeckEntryDisplayWidget(this, deckListing, imageNetworkManager);
        cardListDisplayWidget->setScaleFactor(cardSizeSlider->getSlider()->value());
        connect(cardListDisplayWidget, &ArchidektApiResponseDeckEntryDisplayWidget::requestNavigation, this,
                &ArchidektApiResponseDeckListingsDisplayWidget::requestNavigation);
        connect(cardSizeSlider->getSlider(), &QSlider::valueChanged, cardListDisplayWidget,
                &ArchidektApiResponseDeckEntryDisplayWidget::setScaleFactor);
        flowWidget->addWidget(cardListDisplayWidget);
    }
}

void ArchidektApiResponseDeckListingsDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layout->invalidate();
    layout->activate();
    layout->update();
}
