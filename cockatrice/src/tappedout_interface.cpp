#include "tappedout_interface.h"
#include "decklist.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrlQuery>

TappedOutInterface::TappedOutInterface(
    CardDatabase &_cardDatabase,
    QObject *parent
) : QObject(parent), cardDatabase(_cardDatabase)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(queryFinished(QNetworkReply *)));
}

void TappedOutInterface::queryFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(0, tr("Error"), reply->errorString());
        reply->deleteLater();
        deleteLater();
        return;
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(reply->hasRawHeader("Location"))
    {
        /*
         * If the reply contains a "Location" header, a relative URL to the deck on TappedOut
         * can be extracted from the header. The http status is a 302 "redirect".
         */
        QString deckUrl = reply->rawHeader("Location");
        qDebug() << "Tappedout: good reply, http status" << httpStatus << "location" << deckUrl;
        QDesktopServices::openUrl("http://tappedout.net" + deckUrl);        
    } else {
        /*
         * Otherwise, the deck has not been parsed correctly. Error messages can be extracted
         * from the html. Css pseudo selector for errors: $("div.alert-danger > ul > li")
         */
        QString data(reply->readAll());
        QString errorMessage = tr("Unable to analyze the deck.");
        
        QRegExp rx("<div class=\"alert alert-danger.*<ul>(.*)</ul>");
        rx.setMinimal(true);
        int found = rx.indexIn(data);
        if(found >= 0)
        {
            QString errors = rx.cap(1);
            QRegExp rx2("<li>(.*)</li>");
            rx2.setMinimal(true);

            found = rx2.indexIn(errors);
            int captures = rx2.captureCount();
            for(int i = 1; i <= captures; i++)
            {
                errorMessage += QString("\n") + rx2.cap(i).remove(QRegExp("<[^>]*>")).simplified();
            }

        }

        qDebug() << "Tappedout: bad reply, http status" << httpStatus << "size" << data.size() << "message" << errorMessage;

        QMessageBox::critical(0, tr("Error"), errorMessage);
    }

    reply->deleteLater();
    deleteLater();
}

void TappedOutInterface::getAnalyzeRequestData(DeckList *deck, QByteArray *data)
{
    DeckList mainboard, sideboard;
    copyDeckSplitMainAndSide(*deck, mainboard, sideboard);

    QUrl params;
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("name", deck->getName());
    urlQuery.addQueryItem("mainboard", mainboard.writeToString_Plain(false));
    urlQuery.addQueryItem("sideboard", sideboard.writeToString_Plain(false));
    params.setQuery(urlQuery);
    data->append(params.query(QUrl::EncodeReserved));
}

void TappedOutInterface::analyzeDeck(DeckList *deck)
{
    QByteArray data;
    getAnalyzeRequestData(deck, &data);
    
    QNetworkRequest request(QUrl("http://tappedout.net/mtg-decks/paste/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    manager->post(request, data);
}

struct CopyMainOrSide {
    CardDatabase &cardDatabase;
    DeckList &mainboard, &sideboard;

    CopyMainOrSide(CardDatabase &_cardDatabase, DeckList &_mainboard, DeckList &_sideboard)
     : cardDatabase(_cardDatabase), mainboard(_mainboard), sideboard(_sideboard) {};

    void operator()(const InnerDecklistNode *node, const DecklistCardNode *card) const
    {
        CardInfo * dbCard = cardDatabase.getCard(card->getName());
        if (!dbCard || dbCard->getIsToken())
            return;

        DecklistCardNode *addedCard;
        if(node->getName() == "side")
            addedCard = sideboard.addCard(card->getName(), node->getName());
        else
            addedCard = mainboard.addCard(card->getName(), node->getName());
        addedCard->setNumber(card->getNumber());
    }
};

void TappedOutInterface::copyDeckSplitMainAndSide(const DeckList &source, DeckList &mainboard, DeckList &sideboard)
{
    CopyMainOrSide copyMainOrSide(cardDatabase, mainboard, sideboard);
    source.forEachCard(copyMainOrSide);
}
