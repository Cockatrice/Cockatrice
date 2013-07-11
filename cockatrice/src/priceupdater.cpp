/**
 * @author Marcio Ribeiro <mmr@b1n.org>, Max-Wilhelm Bruker <brukie@gmx.net>
 * @version 1.1
 */
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "qt-json/json.h"
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
    bool ok;
    QVariantMap resultMap = QtJson::Json::parse(QString(reply->readAll()), ok).toMap();
    if (!ok) {
        reply->deleteLater();
        deleteLater();
        return;
    }
    
    QMap<QString, float> cardsPrice;
    
    QListIterator<QVariant> it(resultMap.value("cards").toList());
    while (it.hasNext()) {
        QVariantMap map = it.next().toMap();
        QString name = map.value("name").toString().toLower();
        float price = map.value("price").toString().toFloat();
        QString set = map.value("set_code").toString();

        /**
        * Make sure Masters Edition (MED) isn't the set, as it doesn't
        * physically exist. Also check the price to see that the cheapest set
        * ends up as the final price.
        */
        if (set != "MED" && (!cardsPrice.contains(name) || cardsPrice.value(name) > price))
            cardsPrice.insert(name, price);
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
