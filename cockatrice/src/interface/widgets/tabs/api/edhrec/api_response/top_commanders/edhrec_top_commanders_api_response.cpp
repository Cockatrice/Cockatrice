#include "edhrec_top_commanders_api_response.h"

#include <QDebug>
#include <QJsonArray>

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