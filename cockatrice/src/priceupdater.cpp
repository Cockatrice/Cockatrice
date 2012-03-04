/**
 * @author Marcio Ribeiro <mmr@b1n.org>
 * @version 1.0
 */
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QScriptEngine>
#include <QScriptValueIterator>
#include "priceupdater.h"

/**
 * Constructor.
 *
 * @param _deck deck.
 */
PriceUpdater::PriceUpdater(const DeckList *_deck)
{
    nam = new QNetworkAccessManager(this);
    deck = _deck;
}

/**
 * Update the prices of the cards in deckList.
 */
void PriceUpdater::updatePrices()
{
    QString q = "http://blacklotusproject.com/json/?cards=";
    QStringList cards = deck->getCardList();
    for (int i = 0; i < cards.size(); ++i) {
        q += cards[i] + "|";
    }
    QUrl url(q.replace(' ', '+'));

    QNetworkReply *reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

/**
 * Called when the download of the json file with the prices is finished.
 */
void PriceUpdater::downloadFinished()
{



    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    QByteArray result = reply->readAll();
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("value = " + result);

    QMap<QString, float> cardsPrice;

    if (sc.property("cards").isArray()) {
        QScriptValueIterator it(sc.property("cards"));
        while (it.hasNext()) {
            it.next();
            QString name = it.value().property("name").toString().toLower();
            float price = it.value().property("average").toString().toFloat();
            cardsPrice.insert(name, price);
        }
     }

    InnerDecklistNode *listRoot = deck->getRoot();
    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;
            currentCard->setPrice(cardsPrice[currentCard->getName().toLower()]);
        }
    }

    reply->deleteLater();
    deleteLater();
    emit finishedUpdate();
}
