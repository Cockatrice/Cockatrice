#ifndef DECKDATA_H
#define DECKDATA_H

#include "../archidekt_links/edhrec_api_response_archidekt_links.h"
#include "../cards/edhrec_api_response_card_container.h"
#include "edhrec_commander_api_response_average_deck_statistics.h"

#include <QJsonObject>
#include <QString>

/**
 * @class EdhrecCommanderApiResponse
 * @ingroup ApiResponses
 * @brief Represents the main structure of the JSON
 */
class EdhrecCommanderApiResponse
{
public:
    EdhrecCommanderApiResponseAverageDeckStatistics deckStats;
    EdhrecCommanderApiResponseArchidektLinks archidekt;
    QJsonObject similar;
    QString header;
    QJsonObject panels;
    QString description;
    EdhrecApiResponseCardContainer container;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

#endif // DECKDATA_H
