#include "tapped_out_interface.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

TappedOutInterface::TappedOutInterface(CardDatabase &_cardDatabase, QObject *parent)
    : QObject(parent), cardDatabase(_cardDatabase)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &TappedOutInterface::queryFinished);
}

void TappedOutInterface::queryFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(nullptr, tr("Error"), reply->errorString());
        reply->deleteLater();
        deleteLater();
        return;
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->hasRawHeader("Location")) {
        /*
         * If the reply contains a "Location" header, a relative URL to the deck on TappedOut
         * can be extracted from the header. The http status is a 302 "redirect".
         */
        QString deckUrl = reply->rawHeader("Location");
        qCInfo(TappedOutInterfaceLog) << "Tappedout: good reply, http status" << httpStatus << "location" << deckUrl;
        QDesktopServices::openUrl("https://tappedout.net" + deckUrl);
    } else {
        /*
         * Otherwise, the deck has not been parsed correctly. Error messages can be extracted
         * from the html. Css pseudo selector for errors: $("div.alert-danger > ul > li")
         */
        QString data(reply->readAll());
        QStringList errorMessageList = {tr("Unable to analyze the deck.")};

        static const QRegularExpression rx("<div class=\"alert alert-danger.*?<ul>(.*?)</ul>");
        auto match = rx.match(data);
        if (match.hasMatch()) {
            QString errors = match.captured(1);
            static const QRegularExpression rx2("<li>(.*?)</li>");
            static const QRegularExpression rxremove("<[^>]*>");
            auto matchIterator = rx2.globalMatch(errors);
            while (matchIterator.hasNext()) {
                auto match2 = matchIterator.next();
                errorMessageList.append(match2.captured(1).remove(rxremove).simplified());
            }
        }

        QString errorMessage = errorMessageList.join("\n");
        qCWarning(TappedOutInterfaceLog) << "Tappedout: bad reply, http status" << httpStatus << "size" << data.size()
                                         << "message" << errorMessage;

        QMessageBox::critical(nullptr, tr("Error"), errorMessage);
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
    urlQuery.addQueryItem("mainboard", mainboard.writeToString_Plain(false, true));
    urlQuery.addQueryItem("sideboard", sideboard.writeToString_Plain(false, true));
    params.setQuery(urlQuery);
    data->append(params.query(QUrl::EncodeReserved).toUtf8());
}

void TappedOutInterface::analyzeDeck(DeckList *deck)
{
    QByteArray data;
    getAnalyzeRequestData(deck, &data);

    QNetworkRequest request(QUrl("https://tappedout.net/mtg-decks/paste/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    manager->post(request, data);
}

void TappedOutInterface::copyDeckSplitMainAndSide(DeckList &source, DeckList &mainboard, DeckList &sideboard)
{
    auto copyMainOrSide = [this, &mainboard, &sideboard](const auto node, const auto card) {
        CardInfoPtr dbCard = cardDatabase.query()->getCardInfo(card->getName());
        if (!dbCard || dbCard->getIsToken())
            return;

        DecklistCardNode *addedCard;
        if (node->getName() == DECK_ZONE_SIDE)
            addedCard = sideboard.addCard(card->getName(), node->getName(), -1);
        else
            addedCard = mainboard.addCard(card->getName(), node->getName(), -1);
        addedCard->setNumber(card->getNumber());
    };

    source.forEachCard(copyMainOrSide);
}
