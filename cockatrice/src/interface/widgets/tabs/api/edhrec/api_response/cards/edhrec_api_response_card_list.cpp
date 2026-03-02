#include "edhrec_api_response_card_list.h"

#include <QDebug>

EdhrecApiResponseCardList::EdhrecApiResponseCardList()
{
}

void EdhrecApiResponseCardList::fromJson(const QJsonObject &json)
{
    // Parse the header from the JSON object
    header = json.value("header").toString();

    // Parse the cardviews array and populate cardViews
    QJsonArray cardviewsArray = json.value("cardviews").toArray();
    for (const QJsonValue &value : cardviewsArray) {
        QJsonObject cardviewObj = value.toObject();
        EdhrecApiResponseCardDetails cardView;
        cardView.fromJson(cardviewObj);
        cardViews.append(cardView);
    }
}

void EdhrecApiResponseCardList::debugPrint() const
{
    // Print out the header
    qDebug() << "Header:" << header;

    // Print out all the CardView objects
    for (const EdhrecApiResponseCardDetails &cardView : cardViews) {
        cardView.debugPrint();
    }
}
