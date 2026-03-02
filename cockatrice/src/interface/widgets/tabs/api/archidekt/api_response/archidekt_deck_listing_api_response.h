#ifndef COCKATRICE_ARCHIDEKT_DECK_LISTING_API_RESPONSE_H
#define COCKATRICE_ARCHIDEKT_DECK_LISTING_API_RESPONSE_H

#include "deck_listings/archidekt_api_response_deck_listing_container.h"

#include <QJsonObject>
#include <QString>
#include <QUrl>

class ArchidektDeckListingApiResponse
{

public:
    int count;
    QUrl next;
    QVector<ArchidektApiResponseDeckListingContainer> results;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

#endif // COCKATRICE_DECK_LISTING_API_RESPONSE_H
