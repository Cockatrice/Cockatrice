#include "deck_stats_interface.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/deck_list/tree/inner_deck_list_node.h"

#include <QByteArray>
#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QString>
#include <QTextDocumentFragment>
#include <QUrl>
#include <QWidget>
#include <functional>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/network/network_manager.h>
#include <version_string.h>

namespace
{
bool normalizeAnalysisUrl(QUrl &url, const QString &serviceHost)
{
    const QString host = url.host().toLower();
    const bool legacyHttp = url.scheme() == QStringLiteral("http");
    const int defaultPort = legacyHttp ? 80 : 443;
    if (!url.isValid() || (!legacyHttp && url.scheme() != QStringLiteral("https")) ||
        !url.userInfo().isEmpty() || !url.path().startsWith('/') ||
        (url.port() != -1 && url.port() != defaultPort) ||
        (host != serviceHost && host != QStringLiteral("www.") + serviceHost)) {
        return false;
    }
    // Both services accept HTTPS. Older public metadata may still identify the
    // same deck with HTTP; retain its host/path/query while using HTTPS to open it.
    if (legacyHttp) {
        url.setScheme(QStringLiteral("https"));
        url.setPort(-1);
    }
    return true;
}
} // namespace

DeckStatsInterface::DeckStatsInterface(QObject *parent) : QObject(parent)
{
    manager = NetworkManagerFactory::create(this);
    connect(manager, &QNetworkAccessManager::finished, this, &DeckStatsInterface::queryFinished);
}

void DeckStatsInterface::showError(const QString &message)
{
    // The notice belongs to the originating editor, not this one-shot request.
    // Retire the request before opening UI: opening a window may reenter Qt.
    QWidget *owner = qobject_cast<QWidget *>(parent());
    auto *notice = new QMessageBox(QMessageBox::Critical, tr("Deck analysis"), message, QMessageBox::Ok, owner);
    notice->setTextFormat(Qt::PlainText);
    notice->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    notice->setAttribute(Qt::WA_DeleteOnClose);
    deleteLater();
    notice->open();
}

void DeckStatsInterface::queryFinished(QNetworkReply *reply)
{
    if (replyHandled) {
        return;
    }
    replyHandled = true;
    // All terminal paths release the reply before showing UI or invoking an
    // external URL handler, either of which may destroy the originating editor.
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        showError(reply->errorString());
        return;
    }

    const QString data = QString::fromUtf8(reply->readAll());
    static const QRegularExpression rx(
        R"(<meta\b(?=[^>]*\bproperty\s*=\s*(['"])og:url\1)[^>]*\bcontent\s*=\s*(['"])(.*?)\2)",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const auto match = rx.match(data);
    if (!match.hasMatch()) {
        showError(tr("The reply from Deckstats did not contain a deck address."));
        return;
    }

    const QString address = QTextDocumentFragment::fromHtml(match.captured(3)).toPlainText();
    QUrl deckUrl = reply->url().resolved(QUrl(address, QUrl::StrictMode));
    if (address.trimmed().isEmpty() || !normalizeAnalysisUrl(deckUrl, QStringLiteral("deckstats.net"))) {
        showError(tr("The reply from Deckstats contained an invalid deck address."));
        return;
    }

    QPointer<DeckStatsInterface> guard(this);
    deleteLater();
    if (!QDesktopServices::openUrl(deckUrl) && guard) {
        showError(tr("Could not open your browser. Open this address to see the analyzed deck:\n%1")
                      .arg(deckUrl.toDisplayString()));
    }
}

void DeckStatsInterface::getAnalyzeRequestData(const DeckList &deck, QByteArray &data)
{
    DeckList deckWithoutTokens;
    copyDeckWithoutTokens(deck, deckWithoutTokens);

    data.append("deck=" + QUrl::toPercentEncoding(deckWithoutTokens.writeToString_Plain()));
    data.append("&decktitle=" + QUrl::toPercentEncoding(deck.getName()));
}

void DeckStatsInterface::analyzeDeck(const DeckList &deck)
{
    if (analysisStarted) {
        return;
    }
    analysisStarted = true;
    QByteArray data;
    getAnalyzeRequestData(deck, data);

    QNetworkRequest request(QUrl("https://deckstats.net/index.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    manager->post(request, data);
}

void DeckStatsInterface::copyDeckWithoutTokens(const DeckList &source, DeckList &destination)
{
    auto copyIfNotAToken = [&destination](const auto node, const auto card) {
        CardInfoPtr dbCard = CardDatabaseManager::query()->getCardInfo(card->getName());
        if (dbCard && !dbCard->getIsToken() && node->getName() != DECK_ZONE_MAYBEBOARD) {
            DecklistCardNode *addedCard = destination.addCard(card->getName(), node->getName(), -1);
            addedCard->setNumber(card->getNumber());
        }
    };

    source.forEachCard(copyIfNotAToken);
}
