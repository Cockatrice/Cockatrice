#include "deckstats_interface.h"
#include "decklist.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrlQuery>

DeckStatsInterface::DeckStatsInterface(
    CardDatabase &_cardDatabase,
    QObject *parent
) : QObject(parent), cardDatabase(_cardDatabase)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(queryFinished(QNetworkReply *)));
}

void DeckStatsInterface::queryFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(0, tr("Error"), reply->errorString());
        reply->deleteLater();
        deleteLater();
        return;
    }
        
    QString data(reply->readAll());
    reply->deleteLater();
    
    QRegExp rx("<meta property=\"og:url\" content=\"([^\"]+)\"/>");
    if (-1 == rx.indexIn(data)) {
        QMessageBox::critical(0, tr("Error"), tr("The reply from the server could not be parsed."));
        deleteLater();
        return;
    }
    
    QString deckUrl = rx.cap(1);
    QDesktopServices::openUrl(deckUrl);
    
    deleteLater();
}

void DeckStatsInterface::getAnalyzeRequestData(DeckList *deck, QByteArray *data)
{
    DeckList deckWithoutTokens;
    copyDeckWithoutTokens(*deck, deckWithoutTokens);

    QUrl params;
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("deck", deckWithoutTokens.writeToString_Plain());
    urlQuery.addQueryItem("decktitle", deck->getName());
    params.setQuery(urlQuery);
    data->append(params.query(QUrl::EncodeReserved));
}

void DeckStatsInterface::analyzeDeck(DeckList *deck)
{
    QByteArray data;
    getAnalyzeRequestData(deck, &data);
    
    QNetworkRequest request(QUrl("http://deckstats.net/index.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    manager->post(request, data);
}

struct CopyIfNotAToken {
    CardDatabase &cardDatabase;
    DeckList &destination;

    CopyIfNotAToken(
        CardDatabase &_cardDatabase,
        DeckList &_destination
    ) : cardDatabase(_cardDatabase), destination(_destination) {};

    void operator()(
        const InnerDecklistNode *node,
        const DecklistCardNode *card
    ) const {
        CardInfo * dbCard = cardDatabase.getCard(card->getName());
        if (dbCard && !dbCard->getIsToken()) {
            DecklistCardNode *addedCard = destination.addCard(
                card->getName(),
                node->getName()
            );
            addedCard->setNumber(card->getNumber());
        }
    }
};

void DeckStatsInterface::copyDeckWithoutTokens(
    const DeckList &source,
    DeckList &destination
) {
    CopyIfNotAToken copyIfNotAToken(cardDatabase, destination);
    source.forEachCard(copyIfNotAToken);
}
