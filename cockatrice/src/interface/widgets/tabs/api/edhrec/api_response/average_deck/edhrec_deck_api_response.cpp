#include "edhrec_deck_api_response.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>
#include <libcockatrice/deck_list/deck_loader.h>

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
