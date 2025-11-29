#ifndef EDHREC_AVERAGE_DECK_API_RESPONSE_H
#define EDHREC_AVERAGE_DECK_API_RESPONSE_H

#include "../archidekt_links/edhrec_api_response_archidekt_links.h"
#include "../cards/edhrec_api_response_card_container.h"
#include "../commander/edhrec_commander_api_response_average_deck_statistics.h"
#include "edhrec_deck_api_response.h"

#include <QJsonObject>
#include <QString>

/**
 * @class EdhrecAverageDeckApiResponse
 * @ingroup ApiResponses
 * @brief Represents the main structure of the JSON
 */
class EdhrecAverageDeckApiResponse
{
public:
    EdhrecCommanderApiResponseAverageDeckStatistics deckStats;
    EdhrecCommanderApiResponseArchidektLinks archidekt;
    QJsonObject similar;
    QString header;
    QJsonObject panels;
    QString description;
    EdhrecApiResponseCardContainer container;
    EdhrecDeckApiResponse deck;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

#endif // EDHREC_AVERAGE_DECK_API_RESPONSE_H
