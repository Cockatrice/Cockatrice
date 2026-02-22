#include "commander_spellbook_api_accessor.h"

#include "api_response/commander_spellbook_deck_request.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <version_string.h>

static const QUrl ESTIMATE_BRACKET_URL(QStringLiteral("https://backend.commanderspellbook.com/estimate-bracket"));

CommanderSpellbookApiAccessor &CommanderSpellbookApiAccessor::instance()
{
    static CommanderSpellbookApiAccessor instance;
    return instance;
}

CommanderSpellbookApiAccessor::CommanderSpellbookApiAccessor(QObject *parent) : QObject(parent)
{
}

CommanderSpellbookApiAccessor::RequestId CommanderSpellbookApiAccessor::estimateBracket(const DeckList &deck,
                                                                                        QObject *requester)
{
    CommanderSpellbookDeckRequest deckRequest;
    deckRequest.fromDeckList(deck);

    QJsonDocument doc(deckRequest.toJson());
    QByteArray body = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest req(ESTIMATE_BRACKET_URL);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    QNetworkReply *reply = network.post(req, body);

    const RequestId id = nextRequestId++;

    reply->setProperty("requestId", QVariant::fromValue(id));
    reply->setProperty("requester", QVariant::fromValue(requester));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onEstimateReplyFinished(reply); });

    return id;
}

void CommanderSpellbookApiAccessor::onEstimateReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    const RequestId id = reply->property("requestId").toULongLong();
    QObject *requester = reply->property("requester").value<QObject *>();

    if (!requester) {
        // Requester died — silently drop
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit estimateBracketError(id, requester, reply->errorString());
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit estimateBracketError(id, requester, QStringLiteral("Invalid JSON response"));
        return;
    }

    EstimateBracketResult result;
    result.fromJson(doc.object());

    emit estimateBracketFinished(id, requester, result);
}
