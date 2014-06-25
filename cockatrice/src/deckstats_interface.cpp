#include "deckstats_interface.h"
#include "decklist.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>
#include <QMessageBox>
#include <QDesktopServices>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

DeckStatsInterface::DeckStatsInterface(QObject *parent)
    : QObject(parent)
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
    
    QRegExp rx("id=\"deckstats_deck_url\" value=\"([^\"]+)\"");
    if (!rx.indexIn(data)) {
        QMessageBox::critical(0, tr("Error"), tr("The reply from the server could not be parsed."));
        deleteLater();
        return;
    }
    
    QString deckUrl = rx.cap(1);
    QDesktopServices::openUrl(deckUrl);
    
    deleteLater();
}

#if QT_VERSION < 0x050000
void DeckStatsInterface::getAnalyzeRequestData(DeckList *deck, QByteArray *data)
{
    QUrl params;
    params.addQueryItem("deck", deck->writeToString_Plain());
    data->append(params.encodedQuery());
}
#else
void DeckStatsInterface::getAnalyzeRequestData(DeckList *deck, QByteArray *data)
{
    QUrl params;
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("deck", deck->writeToString_Plain());
    params.setQuery(urlQuery);
    data->append(params.query(QUrl::EncodeReserved));
}
#endif


void DeckStatsInterface::analyzeDeck(DeckList *deck)
{
    QByteArray data;
    getAnalyzeRequestData(deck, &data);
    
    QNetworkRequest request(QUrl("http://deckstats.net/index.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    manager->post(request, data);
}
