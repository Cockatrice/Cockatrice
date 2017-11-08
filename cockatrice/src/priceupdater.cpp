/**
 * @author Marcio Ribeiro <mmr@b1n.org>, Max-Wilhelm Bruker <brukie@gmx.net>
 * @version 1.1
 */
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>

#include "qt-json/json.h"
#include "priceupdater.h"

#include "main.h"
#include "carddatabase.h"

/**
 * Constructor.
 *
 * @param _deck deck.
 */
AbstractPriceUpdater::AbstractPriceUpdater(const DeckList *_deck)
{
    nam = new QNetworkAccessManager(this);
    deck = _deck;
}

// deckbrew.com

/**
 * Constructor.
 *
 * @param _deck deck.
 */

/*
DBPriceUpdater::DBPriceUpdater(const DeckList *_deck)
: AbstractPriceUpdater(_deck)
{
}
*/

/**
 * Update the prices of the cards in deckList.
 */

/*
void DBPriceUpdater::updatePrices()
{
    QString base = "https://api.deckbrew.com/mtg/cards", q = "";
    QStringList cards = deck->getCardList();
    muidMap.clear();
    urls.clear();
    CardInfo * card;
    int muid;
    SetList sets;
    bool bNotFirst=false;

    for (int i = 0; i < cards.size(); ++i) {
        card = db->getCard(cards[i]);
        if(!card)
            continue;
        sets = card->getSets();
        for(int j = 0; j < sets.size(); ++j)
        {
            muid=card->getMuId(sets[j]->getShortName());

            if (!muid) {
                continue;
            }

            //qDebug() << "muid " << muid << " card: " << cards[i] << endl;
            if(bNotFirst)
            {
                q += QString("&m=%1").arg(muid);
            } else {
                q += QString("?m=%1").arg(muid);
                bNotFirst = true;
            }
            muidMap.insert(muid, cards[i]);

            if(q.length() > 240)
            {
                urls.append(base + q);
                bNotFirst=false;
                q = "";
            }
        }
    }
    if(q.length() > 0)
        urls.append(base + q);

    requestNext();
}
*/

/*
void DBPriceUpdater::requestNext()
{
    if(urls.empty())
        return;

    QUrl url(urls.takeFirst(), QUrl::TolerantMode);
    //qDebug() << "request prices from: " << url.toString() << endl;
    QNetworkReply *reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}
*/

/**
 * Called when the download of the json file with the prices is finished.
 */
/*
void DBPriceUpdater::downloadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    bool ok;
    QString tmp = QString(reply->readAll());

    // Errors are incapsulated in an object, check for them first
    QVariantMap resultMap = QtJson::Json::parse(tmp, ok).toMap();
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("A problem has occured while fetching card prices."));
        reply->deleteLater();
        if(urls.isEmpty())
        {
            deleteLater();
            emit finishedUpdate();
        } else {
            requestNext();
        }
    }

    if(resultMap.contains("errors"))
    {
        QMessageBox::critical(this, tr("Error"), tr("A problem has occured while fetching card prices:") + 
            "<br/>" + resultMap["errors"].toList().first().toString().toHtmlEscaped()
        );
        reply->deleteLater();
        if(urls.isEmpty())
        {
            deleteLater();
            emit finishedUpdate();
        } else {
            requestNext();
        }
    }

    // Good results are a list
    QVariantList resultList = QtJson::Json::parse(tmp, ok).toList();
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("A problem has occured while fetching card prices."));
        reply->deleteLater();
        if(urls.isEmpty())
        {
            deleteLater();
            emit finishedUpdate();
        } else {
            requestNext();
        }
    }

    QMap<QString, float> cardsPrice;
    QListIterator<QVariant> it(resultList);
    while (it.hasNext()) {
        QVariantMap cardMap = it.next().toMap();


        // get sets list
        QList<QVariant> editions = cardMap.value("editions").toList();
        foreach (QVariant ed, editions)
        {
            // retrieve card name "as we know it" from the muid
            QVariantMap edition = ed.toMap();
            QString set = edition.value("set_id").toString();

            int muid = edition.value("multiverse_id").toString().toInt();
            if(!muidMap.contains(muid))
                continue;

            QString name=muidMap.value(muid);
            // Prices are in USD cents
            float price = edition.value("price").toMap().value("median").toString().toFloat() / 100;
            //qDebug() << "card " << name << " set " << set << " price " << price << endl;


            * Make sure Masters Edition (MED) isn't the set, as it doesn't
            * physically exist. Also check the price to see that the cheapest set
            * ends up as the final price.

            if (set != "MED" && price > 0 && (!cardsPrice.contains(name) || cardsPrice.value(name) > price))
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
            float price = cardsPrice[currentCard->getName()];
            if(price > 0)
                currentCard->setPrice(price);
        }
    }
    
    reply->deleteLater();
    if(urls.isEmpty())
    {
        deleteLater();
        emit finishedUpdate();
    } else {
        requestNext();
    }
}
*/
