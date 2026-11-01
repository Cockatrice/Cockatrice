#include "tapped_out_interface.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/deck_list/tree/inner_deck_list_node.h"

#include <QByteArray>
#include <QDebug>
#include <QDesktopServices>
#include <QList>
#include <QMessageBox>
#include <QMessageLogger>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTextDocumentFragment>
#include <QUrl>
#include <QVariant>
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

TappedOutInterface::TappedOutInterface(QObject *parent) : QObject(parent)
{
    manager = NetworkManagerFactory::create(this);
    connect(manager, &QNetworkAccessManager::finished, this, &TappedOutInterface::queryFinished);
}

void TappedOutInterface::showError(const QString &message)
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

void TappedOutInterface::queryFinished(QNetworkReply *reply)
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

    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStatus >= 200 && httpStatus < 400 && reply->hasRawHeader("Location")) {
        const QByteArray location = reply->rawHeader("Location");
        QUrl deckUrl = reply->url().resolved(QUrl::fromEncoded(location, QUrl::StrictMode));
        if (location.trimmed().isEmpty() || !normalizeAnalysisUrl(deckUrl, QStringLiteral("tappedout.net"))) {
            showError(tr("The reply from TappedOut contained an invalid deck address."));
            return;
        }
        QPointer<TappedOutInterface> guard(this);
        deleteLater();
        if (!QDesktopServices::openUrl(deckUrl) && guard) {
            showError(tr("Could not open your browser. Open this address to see the analyzed deck:\n%1")
                          .arg(deckUrl.toDisplayString()));
        }
        return;
    }

    const QString data = QString::fromUtf8(reply->readAll());
    QStringList errorMessageList = {tr("Unable to analyze the deck on TappedOut.")};
    static const QRegularExpression errorsRx(
        "<div class=\"alert alert-danger.*?<ul>(.*?)</ul>", QRegularExpression::DotMatchesEverythingOption);
    const auto match = errorsRx.match(data);
    if (match.hasMatch()) {
        static const QRegularExpression itemRx("<li>(.*?)</li>", QRegularExpression::DotMatchesEverythingOption);
        auto items = itemRx.globalMatch(match.captured(1));
        while (items.hasNext()) {
            const QString error = QTextDocumentFragment::fromHtml(items.next().captured(1)).toPlainText().simplified();
            if (!error.isEmpty()) {
                errorMessageList.append(error);
            }
        }
    }
    qCWarning(TappedOutInterfaceLog) << "TappedOut analysis failed, HTTP status" << httpStatus;
    showError(errorMessageList.join('\n'));
}

void TappedOutInterface::getAnalyzeRequestData(const DeckList &deck, QByteArray &data)
{
    DeckList mainboard, sideboard;
    copyDeckSplitMainAndSide(deck, mainboard, sideboard);

    data.append("name=" + QUrl::toPercentEncoding(deck.getName()));
    data.append("&mainboard=" + QUrl::toPercentEncoding(mainboard.writeToString_Plain(false, true)));
    data.append("&sideboard=" + QUrl::toPercentEncoding(sideboard.writeToString_Plain(false, true)));
}

void TappedOutInterface::analyzeDeck(const DeckList &deck)
{
    if (analysisStarted) {
        return;
    }
    analysisStarted = true;
    QByteArray data;
    getAnalyzeRequestData(deck, data);

    QNetworkRequest request(QUrl("https://tappedout.net/mtg-decks/paste/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    // we interpret the redirect and open it in the browser instead, do not follow redirects
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

    manager->post(request, data);
}

void TappedOutInterface::copyDeckSplitMainAndSide(const DeckList &source, DeckList &mainboard, DeckList &sideboard)
{
    auto copyMainOrSide = [&mainboard, &sideboard](const auto node, const auto card) {
        CardInfoPtr dbCard = CardDatabaseManager::query()->getCardInfo(card->getName());
        if (!dbCard || dbCard->getIsToken() || node->getName() == DECK_ZONE_MAYBEBOARD) {
            return;
        }

        DecklistCardNode *addedCard;
        if (node->getName() == DECK_ZONE_SIDE) {
            addedCard = sideboard.addCard(card->getName(), node->getName(), -1);
        } else {
            addedCard = mainboard.addCard(card->getName(), node->getName(), -1);
        }
        addedCard->setNumber(card->getNumber());
    };

    source.forEachCard(copyMainOrSide);
}
