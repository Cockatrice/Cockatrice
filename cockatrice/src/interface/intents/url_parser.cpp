#include "url_parser.h"

#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "../window_main.h"
#include "contexts/context_join_game.h"
#include "intent_join_server_game.h"
#include "intent_login.h"

#include <QDebug>
#include <QMessageBox>
#include <QUrl>
#include <QUrlQuery>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <memory>

IntentUrlParser::IntentUrlParser(QObject *parent, MainWindow *_mainWindow) : QObject(parent), mainWindow(_mainWindow)
{
}

void IntentUrlParser::handle(const QString &urlStr)
{
    QUrl url(urlStr);

    if (url.scheme() != "cockatrice") {
        return;
    }

    const QString action = url.host();
    QUrlQuery query(url);

    if (action == "joingame") {
        handleJoinGame(query);
    } else if (action == "opendeck") {
        // handleOpenDeck(query);
    } else {
        qWarning() << "Unknown intent:" << action;
    }
}

void IntentUrlParser::handleJoinGame(const QUrlQuery &query)
{
    auto showError = [this](const QString &message) { QMessageBox::warning(mainWindow, tr("Open game"), message); };

    auto ctx = std::make_unique<ContextJoinGame>();

    ctx->roomContext.serverContext.hostname = query.queryItemValue("hostname");
    ctx->roomContext.serverContext.port = query.queryItemValue("port");

    if (ctx->roomContext.serverContext.hostname.isEmpty()) {
        showError(tr("Missing or empty hostname in the game link"));
        return;
    }

    bool ok = false;
    ctx->roomContext.serverContext.port.toUShort(&ok);
    if (!ok) {
        showError(tr("Invalid or missing port in the game link"));
        return;
    }

    ctx->roomContext.roomId = query.queryItemValue("roomid").toInt(&ok);

    if (!ok) {
        showError(tr("Invalid or missing room id in the game link"));
        return;
    }

    ok = false;
    ctx->gameId = query.queryItemValue("gameid").toInt(&ok);

    if (!ok) {
        showError(tr("Invalid or missing game id in the game link"));
        return;
    }

    const QString hostname = ctx->roomContext.serverContext.hostname;
    const QString port = ctx->roomContext.serverContext.port;
    const int roomId = ctx->roomContext.roomId;
    const int gameId = ctx->gameId;
    const QString server = QStringLiteral("%1:%2").arg(hostname, port);

    // Prefer the room name over the raw numeric id: it means something to the
    // user. The name is only known when we are already connected to the same
    // server and sitting in that room — otherwise fall back to a plain prompt.
    AbstractClient *client = mainWindow->getTabSupervisor()->getClient();
    const bool sameServer = client != nullptr && client->getStatus() == StatusLoggedIn &&
                            hostname.compare(client->serverName(), Qt::CaseInsensitive) == 0 &&
                            QString::number(client->serverPort()) == port;
    TabRoom *roomTab = sameServer ? mainWindow->getTabSupervisor()->getRoomTabs().value(roomId) : nullptr;

    // Links built by newer clients embed the game description ("game" item);
    // restate it in the confirm so it matches what the chat anchor showed.
    // Unknown query items are ignored, so old links without it keep working.
    // The multi-arg .arg() overloads replace in a single pass, so a description
    // containing "%…" cannot corrupt later placeholders.
    // FullyDecoded undoes every %XX escape and must match the chat anchor's
    // decode mode, so a description containing "%" reads identically in both.
    const QString gameDescription = query.queryItemValue("game", QUrl::FullyDecoded);
    const QString gameIdStr = QString::number(gameId);
    const QString message =
        gameDescription.isEmpty()
            ? (roomTab ? tr("Join game #%1 in \"%2\" on %3?").arg(gameIdStr, roomTab->getRoomName(), server)
                       : tr("Join game #%1 on %2?").arg(gameIdStr, server))
            : (roomTab ? tr("Join game \"%1\" (#%2) in \"%3\" on %4?")
                             .arg(gameDescription, gameIdStr, roomTab->getRoomName(), server)
                       : tr("Join game \"%1\" (#%2) on %3?").arg(gameDescription, gameIdStr, server));

    const QMessageBox::StandardButton answer = QMessageBox::question(
        mainWindow, tr("Join game"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer != QMessageBox::Yes) {
        return;
    }

    // The join game intent owns the context and the credential lookup; once the
    // chain finishes (or fails) it deletes the whole tree.
    ContextConnectToServer *serverContext = &ctx->roomContext.serverContext;
    auto joinGameIntent =
        new IntentJoinServerGame(mainWindow->getTabSupervisor(), mainWindow->getRemoteClient(), std::move(ctx));
    joinGameIntent->setParent(this);

    auto getLoginCredentialsIntent = new IntentGetLoginCredentials(serverContext);
    getLoginCredentialsIntent->setParent(joinGameIntent);

    connect(getLoginCredentialsIntent, &Intent::finished, joinGameIntent, &Intent::execute);
    connect(getLoginCredentialsIntent, &Intent::failed, joinGameIntent, &Intent::failed);
    connect(joinGameIntent, &Intent::failed, this, [showError](const QString &reason) { showError(reason); });

    getLoginCredentialsIntent->execute();
}
