#include "edhrec_commander_api_response_card_container.h"

#include <QJsonArray>
#include <QJsonObject>

void EdhrecCommanderApiResponseCardContainer::fromJson(const QJsonObject &json)
{
    // Parse breadcrumb
    QJsonArray breadcrumbArray = json.value("breadcrumb").toArray();
    for (const QJsonValue &breadcrumbValue : breadcrumbArray) {
        breadcrumb.push_back(breadcrumbValue.toObject());
    }

    description = json.value("description").toString();
    QJsonObject jsonDict = json.value("json_dict").toObject();
    card.fromJson(jsonDict.value("card").toObject());
    QJsonArray cardlistsArray = jsonDict.value("cardlists").toArray();

    for (const QJsonValue &cardlistValue : cardlistsArray) {
        QJsonObject cardlistObj = cardlistValue.toObject();
        QJsonArray cardviewsArray = cardlistObj.value("cardviews").toArray();
        EdhrecCommanderApiResponseCardList cardView;
        cardView.fromJson(cardlistValue.toObject());
        cardlists.push_back(cardView);
    }

    keywords = json.value("keywords").toString();
    title = json.value("title").toString();
}

void EdhrecCommanderApiResponseCardContainer::debugPrint() const
{
    qDebug() << "Breadcrumb:";
    for (const auto &breadcrumbEntry : breadcrumb) {
        qDebug() << breadcrumbEntry;
    }

    qDebug() << "Description:" << description;
    card.debugPrint();

    qDebug() << "Cardlists:";
    for (const auto &cardlist : cardlists) {
        cardlist.debugPrint();
    }

    qDebug() << "Keywords:" << keywords;
    qDebug() << "Title:" << title;
}
