#ifndef EDHREC_TOP_COMMANDERS_API_RESPONSE_H
#define EDHREC_TOP_COMMANDERS_API_RESPONSE_H

#include "../commander/edhrec_commander_api_response_card_container.h"

#include <QDebug>
#include <QJsonObject>
#include <QString>

// Represents the main structure of the JSON
class EdhrecTopCommandersApiResponse
{
public:
    QString header;
    QString description;
    EdhrecCommanderApiResponseCardContainer container;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

#endif //EDHREC_TOP_COMMANDERS_API_RESPONSE_H
