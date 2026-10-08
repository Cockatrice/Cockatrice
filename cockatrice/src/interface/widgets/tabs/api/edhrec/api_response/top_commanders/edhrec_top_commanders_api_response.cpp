#include "edhrec_top_commanders_api_response.h"

#include "../cards/edhrec_api_response_card_container.h"

#include <QDebug>
#include <QJsonValue>
#include <qlogging.h>

void EdhrecTopCommandersApiResponse::fromJson(const QJsonObject &json)
{
    header = json.value("header").toString();
    description = json.value("description").toString();
    QJsonObject containerJson = json.value("container").toObject();
    container.fromJson(containerJson);
}

void EdhrecTopCommandersApiResponse::debugPrint() const
{
    qDebug() << "Header:" << header;
    qDebug() << "Description:" << description;
    container.debugPrint();
}