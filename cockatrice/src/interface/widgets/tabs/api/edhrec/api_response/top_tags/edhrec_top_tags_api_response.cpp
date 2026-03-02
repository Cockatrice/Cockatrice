#include "edhrec_top_tags_api_response.h"

#include <QDebug>
#include <QJsonArray>

void EdhrecTopTagsApiResponse::fromJson(const QJsonObject &json)
{
    header = json.value("header").toString();
    description = json.value("description").toString();
    QJsonObject containerJson = json.value("container").toObject();
    container.fromJson(containerJson);
}

void EdhrecTopTagsApiResponse::debugPrint() const
{
    qDebug() << "Header:" << header;
    qDebug() << "Description:" << description;
    container.debugPrint();
}