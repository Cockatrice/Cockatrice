#include "edhrec_deck_api_response.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonValueConstRef>
#include <QString>
#include <QTextStream>
#include <functional>
#include <libcockatrice/card/import/card_name_normalizer.h>
#include <qlogging.h>

void EdhrecDeckApiResponse::fromJson(const QJsonArray &json)
{
    QString deckList;
    for (const QJsonValue &cardlistValue : json) {
        deckList += cardlistValue.toString() + "\n";
    }

    QTextStream stream(&deckList);
    deck.loadFromStream_Plain(stream, true, CardNameNormalizer());
}

void EdhrecDeckApiResponse::debugPrint() const
{
    qDebug() << "Breadcrumb:";
}
