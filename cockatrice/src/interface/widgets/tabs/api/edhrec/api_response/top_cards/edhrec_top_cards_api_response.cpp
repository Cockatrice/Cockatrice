#include "edhrec_top_cards_api_response.h"

#include <QDebug>
#include <QJsonArray>

void EdhrecTopCardsApiResponse::fromJson(const QJsonObject &json)
{
    header = json.value("header").toString();
    description = json.value("description").toString();
    QJsonObject containerJson = json.value("container").toObject();
    container.fromJson(containerJson);
}

void EdhrecTopCardsApiResponse::debugPrint() const
{
    qDebug() << "Header:" << header;
    qDebug() << "Description:" << description;
    container.debugPrint();
}