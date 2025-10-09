/**
 * @file edhrec_top_commanders_api_response.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef EDHREC_TOP_COMMANDERS_API_RESPONSE_H
#define EDHREC_TOP_COMMANDERS_API_RESPONSE_H

#include "../cards/edhrec_api_response_card_container.h"

#include <QDebug>
#include <QString>

class EdhrecTopCommandersApiResponse
{
public:
    QString header;
    QString description;
    EdhrecApiResponseCardContainer container;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

#endif // EDHREC_TOP_COMMANDERS_API_RESPONSE_H
