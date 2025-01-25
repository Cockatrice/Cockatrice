#ifndef DECKDATA_H
#define DECKDATA_H

#include "edhrec_commander_api_response_archidekt_links.h"
#include "edhrec_commander_api_response_average_deck_statistics.h"
#include "edhrec_commander_api_response_card_container.h"

#include <QDebug>
#include <QJsonObject>
#include <QString>

// Represents the main structure of the JSON
class EdhrecCommanderApiResponse
{
public:
    EdhrecCommanderApiResponseAverageDeckStatistics deckStats;
    EdhrecCommanderApiResponseArchidektLinks archidekt;
    QJsonObject similar;
    QString header;
    QJsonObject panels;
    QString description;
    EdhrecCommanderApiResponseCardContainer container;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

#endif // DECKDATA_H
