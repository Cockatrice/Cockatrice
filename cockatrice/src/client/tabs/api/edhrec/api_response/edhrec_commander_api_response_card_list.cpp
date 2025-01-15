#include "edhrec_commander_api_response_card_list.h"
#include <QDebug>

EdhrecCommanderApiResponseCardList::EdhrecCommanderApiResponseCardList()
{
}

void EdhrecCommanderApiResponseCardList::fromJson(const QJsonObject &json)
{
    // Parse the header from the JSON object
    header = json.value("header").toString();

    // Parse the cardviews array and populate cardViews
    QJsonArray cardviewsArray = json.value("cardviews").toArray();
    for (const QJsonValue &value : cardviewsArray) {
        QJsonObject cardviewObj = value.toObject();
        EdhrecCommanderApiResponseCardDetails cardView;
        cardView.fromJson(cardviewObj);
        cardViews.append(cardView);
    }
}

void EdhrecCommanderApiResponseCardList::debugPrint() const
{
    // Print out the header
    qDebug() << "Header:" << header;

    // Print out all the CardView objects
    for (const EdhrecCommanderApiResponseCardDetails &cardView : cardViews) {
        cardView.debugPrint();
    }
}
