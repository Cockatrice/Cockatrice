#include "deck_stats_interface.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <version_string.h>

DeckStatsInterface::DeckStatsInterface(CardDatabase &_cardDatabase, QObject *parent)
    : QObject(parent), cardDatabase(_cardDatabase)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &DeckStatsInterface::queryFinished);
}

void DeckStatsInterface::queryFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(nullptr, tr("Error"), reply->errorString());
        reply->deleteLater();
        deleteLater();
        return;
    }

    QString data(reply->readAll());
    reply->deleteLater();

    static const QRegularExpression rx("<meta property=\"og:url\" content=\"([^\"]+)\"");
    auto match = rx.match(data);
    if (!match.hasMatch()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("The reply from the server could not be parsed."));
        deleteLater();
        return;
    }

    QString deckUrl = match.captured(1);
    QDesktopServices::openUrl(deckUrl);

    deleteLater();
}

void DeckStatsInterface::getAnalyzeRequestData(const DeckList &deck, QByteArray &data)
{
    DeckList deckWithoutTokens;
    copyDeckWithoutTokens(deck, deckWithoutTokens);

    QUrl params;
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("deck", deckWithoutTokens.writeToString_Plain());
    urlQuery.addQueryItem("decktitle", deck.getName());
    params.setQuery(urlQuery);
    data.append(params.query(QUrl::EncodeReserved).toUtf8());
}

void DeckStatsInterface::analyzeDeck(const DeckList &deck)
{
    QByteArray data;
    getAnalyzeRequestData(deck, data);

    QNetworkRequest request(QUrl("https://deckstats.net/index.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    manager->post(request, data);
}

void DeckStatsInterface::copyDeckWithoutTokens(const DeckList &source, DeckList &destination)
{
    auto copyIfNotAToken = [this, &destination](const auto node, const auto card) {
        CardInfoPtr dbCard = cardDatabase.query()->getCardInfo(card->getName());
        if (dbCard && !dbCard->getIsToken()) {
            DecklistCardNode *addedCard = destination.addCard(card->getName(), node->getName(), -1);
            addedCard->setNumber(card->getNumber());
        }
    };

    source.forEachCard(copyIfNotAToken);
}
