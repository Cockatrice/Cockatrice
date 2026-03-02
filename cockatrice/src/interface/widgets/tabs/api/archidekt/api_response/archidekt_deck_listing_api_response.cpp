#include "archidekt_deck_listing_api_response.h"

#include <QDebug>
#include <QJsonArray>

void ArchidektDeckListingApiResponse::fromJson(const QJsonObject &json)
{
    count = json.value("count").toInt();
    next = QUrl(json.value("next").toString());

    QJsonArray containerJson = json.value("results").toArray();

    for (const QJsonValue &deckListingValue : containerJson) {
        ArchidektApiResponseDeckListingContainer listingResult;
        listingResult.fromJson(deckListingValue.toObject());
        results.append(listingResult);
    }
}

void ArchidektDeckListingApiResponse::debugPrint() const
{
    qDebug() << "Count:" << count;
    qDebug() << "Next:" << next;

    qDebug() << "Results:";
    for (const auto &deckListing : results) {
        deckListing.debugPrint();
    }
}