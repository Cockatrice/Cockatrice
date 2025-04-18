#include "edhrec_deck_api_response.h"

#include "../../../../../../deck/deck_loader.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>

void EdhrecDeckApiResponse::fromJson(const QJsonArray &json)
{
    QString deckList;
    for (const QJsonValue &cardlistValue : json) {
        deckList += cardlistValue.toString() + "\n";
    }

    deckLoader = new DeckLoader();

    QTextStream stream(&deckList);
    deckLoader->loadFromStream_Plain(stream, true);
}

void EdhrecDeckApiResponse::debugPrint() const
{
    qDebug() << "Breadcrumb:";
}
