#include "edhrec_deck_api_response.h"

#include "../../../../../../deck_loader/deck_loader.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

void EdhrecDeckApiResponse::fromJson(const QJsonArray &json)
{
    QString deckList;
    for (const QJsonValue &cardlistValue : json) {
        deckList += cardlistValue.toString() + "\n";
    }

    deckLoader = new DeckLoader(nullptr);

    QTextStream stream(&deckList);
    deckLoader->getDeckList()->loadFromStream_Plain(stream, true);
}

void EdhrecDeckApiResponse::debugPrint() const
{
    qDebug() << "Breadcrumb:";
}
