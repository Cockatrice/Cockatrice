#include "url_parser.h"

#include "../../client/settings/cache_settings.h"
#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "../window_main.h"
#include "contexts/context_join_game.h"
#include "contexts/context_open_deck.h"
#include "intent.h"
#include "intent_join_server_game.h"
#include "intent_login.h"
#include "intent_open_shared_deck.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QUrl>
#include <QUrlQuery>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/settings/servers_settings.h>
#include <memory>

inline Q_LOGGING_CATEGORY(UrlParserLog, "url_parser");

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

    qCDebug(UrlParserLog) << "Parsing intent URL, action:" << action;

    QList<Intent *> chain;
    Intent *firstIntent = nullptr;
    if (action == "joingame") {
        firstIntent = createJoinGameIntent(query, chain);
    } else if (action == "opendeck") {
        firstIntent = createOpenDeckIntent(query, chain);
    } else {
        qWarning() << "Unknown intent:" << action;
    }

    if (firstIntent == nullptr) {
        // The link was invalid or the user declined the confirm: nothing runs.
        // Report the idle state when no other chain is queued so that a startup
        // launch (which skipped its own connection for this URL) falls back to it.
        if (!chainRunning && pendingChains.isEmpty()) {
            emit urlChainFinished(mainWindow->getRemoteClient()->getStatus() == StatusLoggedIn);
        }
        return;
    }

    pendingChains.append(chain);
    startNextChain();
}

Intent *IntentUrlParser::createJoinGameIntent(const QUrlQuery &query, QList<Intent *> &chain)
{
    auto showError = [this](const QString &message) { QMessageBox::warning(mainWindow, tr("Open game"), message); };

    auto ctx = std::make_unique<ContextJoinGame>();

    ctx->roomContext.serverContext.hostname = query.queryItemValue("hostname");
    ctx->roomContext.serverContext.port = query.queryItemValue("port");

    if (ctx->roomContext.serverContext.hostname.isEmpty()) {
        showError(tr("Missing or empty hostname in the game link"));
        return nullptr;
    }

    bool ok = false;
    ctx->roomContext.serverContext.port.toUShort(&ok);
    if (!ok) {
        showError(tr("Invalid or missing port in the game link"));
        return nullptr;
    }

    ctx->roomContext.roomId = query.queryItemValue("roomid").toInt(&ok);

    if (!ok) {
        showError(tr("Invalid or missing room id in the game link"));
        return nullptr;
    }

    ok = false;
    ctx->gameId = query.queryItemValue("gameid").toInt(&ok);

    if (!ok) {
        showError(tr("Invalid or missing game id in the game link"));
        return nullptr;
    }

    const QString gameDescription = query.queryItemValue("game", QUrl::FullyDecoded);
    const QString message = generateJoinGameMessage(*ctx, gameDescription);

    const QMessageBox::StandardButton answer = QMessageBox::question(
        mainWindow, tr("Join game"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer != QMessageBox::Yes) {
        return nullptr;
    }

    RemoteClient *client = mainWindow->getRemoteClient();
    ContextConnectToServer *serverContext = &ctx->roomContext.serverContext;

    // The join game intent owns the context and the credential lookup; once the
    // chain finishes (or fails) it deletes the whole tree.
    auto joinGameIntent = new IntentJoinServerGame(mainWindow->getTabSupervisor(), client, std::move(ctx));
    joinGameIntent->setParent(this);
    chain.append(joinGameIntent);
    connect(joinGameIntent, &Intent::failed, this, [showError](const QString &reason) { showError(reason); });

    Intent *firstIntent = joinGameIntent;
    if (!isConnectedTo(serverContext->hostname, serverContext->port)) {
        auto getLoginCredentialsIntent = new IntentGetLoginCredentials(serverContext);
        getLoginCredentialsIntent->setParent(joinGameIntent);
        chain.insert(0, getLoginCredentialsIntent);

        connect(getLoginCredentialsIntent, &Intent::finished, joinGameIntent, &Intent::execute);
        connect(getLoginCredentialsIntent, &Intent::failed, joinGameIntent, &Intent::failed);
        connect(getLoginCredentialsIntent, &Intent::cancelled, joinGameIntent, &Intent::cancelled);
        firstIntent = getLoginCredentialsIntent;
    }

    return firstIntent;
}

QString IntentUrlParser::generateJoinGameMessage(const ContextJoinGame &context, const QString &gameDescription)
{
    const QString hostname = context.roomContext.serverContext.hostname;
    const QString port = context.roomContext.serverContext.port;
    const int roomId = context.roomContext.roomId;
    const int gameId = context.gameId;
    const QString server = QStringLiteral("%1:%2").arg(hostname, port);

    // Prefer the room name over the raw numeric id: it means something to the
    // user. The name is only known when we are already connected to the same
    // server and sitting in that room — otherwise fall back to a plain prompt.
    AbstractClient *client = mainWindow->getTabSupervisor()->getClient();
    const bool sameServer = client != nullptr && client->getStatus() == StatusLoggedIn &&
                            hostname.compare(client->serverName(), Qt::CaseInsensitive) == 0 &&
                            QString::number(client->serverPort()) == port;
    TabRoom *roomTab = sameServer ? mainWindow->getTabSupervisor()->getRoomTabs().value(roomId) : nullptr;

    const QString gameIdStr = QString::number(gameId);
    // Links built by newer clients embed the game description ("game" item);
    // restate it in the confirm so it matches what the chat anchor showed.
    // Unknown query items are ignored, so old links without it keep working.
    // The multi-arg .arg() overloads replace in a single pass, so a description
    // containing "%…" cannot corrupt later placeholders.
    // FullyDecoded undoes every %XX escape and must match the chat anchor's
    // decode mode, so a description containing "%" reads identically in both.
    if (gameDescription.isEmpty()) {
        return roomTab ? tr("Join game #%1 in \"%2\" on %3?").arg(gameIdStr, roomTab->getRoomName(), server)
                       : tr("Join game #%1 on %2?").arg(gameIdStr, server);
    }

    return roomTab ? tr("Join game \"%1\" (#%2) in \"%3\" on %4?")
                         .arg(gameDescription, gameIdStr, roomTab->getRoomName(), server)
                   : tr("Join game \"%1\" (#%2) on %3?").arg(gameDescription, gameIdStr, server);
}

Intent *IntentUrlParser::createOpenDeckIntent(const QUrlQuery &query, QList<Intent *> &chain)
{
    auto showError = [this](const QString &message) {
        QMessageBox::warning(mainWindow, tr("Open shared deck"), message);
    };

    auto ctx = std::make_unique<ContextOpenDeck>();

    ctx->serverContext.hostname = query.queryItemValue("hostname");
    ctx->serverContext.port = query.queryItemValue("port");
    ctx->shareToken = query.queryItemValue("share");

    qCDebug(UrlParserLog) << "Open-deck intent: host" << ctx->serverContext.hostname << "port"
                          << ctx->serverContext.port << "token length" << ctx->shareToken.length();

    if (ctx->serverContext.hostname.isEmpty()) {
        showError(tr("Missing or empty hostname in the share link"));
        return nullptr;
    }

    bool ok = false;
    const quint16 port = ctx->serverContext.port.toUShort(&ok);
    if (!ok || port == 0) {
        showError(tr("Invalid or missing port in the share link"));
        return nullptr;
    }

    if (ctx->shareToken.isEmpty()) {
        showError(tr("Missing or empty share value in the share link"));
        return nullptr;
    }

    RemoteClient *client = mainWindow->getRemoteClient();

    // When the link would move us away from a live session, ask first — the
    // open deck download needs the connection the user already has. Remember
    // the current session so a failed or cancelled chain can restore it.
    const bool migrating =
        client->getStatus() == StatusLoggedIn && !isConnectedTo(ctx->serverContext.hostname, ctx->serverContext.port);
    if (migrating) {
        const QString target = QStringLiteral("%1:%2").arg(ctx->serverContext.hostname, ctx->serverContext.port);
        const QString current =
            QStringLiteral("%1:%2").arg(client->serverName(), QString::number(client->serverPort()));
        const QMessageBox::StandardButton answer = QMessageBox::question(
            mainWindow, tr("Open shared deck"),
            tr("Opening this share link connects you to %1 instead of %2.\n\nContinue?").arg(target, current),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (answer != QMessageBox::Yes) {
            return nullptr;
        }
        migrationTargetHost = ctx->serverContext.hostname;
        migrationTargetPort = ctx->serverContext.port;
        previousServerHost = client->serverName();
        previousServerPort = QString::number(client->serverPort());
        pendingRestore = true;
    }

    ContextConnectToServer *serverContext = &ctx->serverContext;

    // The open deck intent owns the context and the credential lookup; once
    // the chain finishes (or fails) it deletes the whole tree.
    auto openDeckIntent =
        new IntentOpenSharedDeck(mainWindow->getTabSupervisor(), client, CardDatabaseManager::query(), std::move(ctx));
    openDeckIntent->setParent(this);
    chain.append(openDeckIntent);
    connect(openDeckIntent, &Intent::failed, this, [showError](const QString &reason) { showError(reason); });

    Intent *firstIntent = openDeckIntent;
    if (!isConnectedTo(serverContext->hostname, serverContext->port)) {
        auto getLoginCredentialsIntent = new IntentGetLoginCredentials(serverContext);
        getLoginCredentialsIntent->setParent(openDeckIntent);
        chain.insert(0, getLoginCredentialsIntent);

        connect(getLoginCredentialsIntent, &Intent::finished, openDeckIntent, &Intent::execute);
        connect(getLoginCredentialsIntent, &Intent::failed, openDeckIntent, &Intent::failed);
        connect(getLoginCredentialsIntent, &Intent::cancelled, openDeckIntent, &Intent::cancelled);
        firstIntent = getLoginCredentialsIntent;
    }

    return firstIntent;
}

bool IntentUrlParser::isConnectedTo(const QString &hostname, const QString &port) const
{
    Q_UNUSED(port);
    // Deliberately hostname-only (no port): the intents' preconditions apply the
    // same rule, so a link to the same host on another port still connects
    // rather than silently reusing an existing session on a different server.
    RemoteClient *client = mainWindow->getRemoteClient();
    return client->getStatus() == StatusLoggedIn && client->serverName().compare(hostname, Qt::CaseInsensitive) == 0;
}

void IntentUrlParser::startNextChain()
{
    if (chainRunning || pendingChains.isEmpty()) {
        return;
    }
    chainRunning = true;
    currentChainSucceeded = false;

    const QList<Intent *> chain = pendingChains.takeFirst();
    if (chain.isEmpty()) {
        chainRunning = false;
        return;
    }

    // Only the last intent completes the chain; its terminal signal ends the
    // whole run. Cancellation of an intermediate intent (e.g. declined login
    // prompt) is forwarded onto the last intent in the chain builders above.
    Intent *finalIntent = chain.last();
    connect(finalIntent, &Intent::finished, this, [this]() {
        currentChainSucceeded = true;
        chainEnded();
    });
    connect(finalIntent, &Intent::failed, this, &IntentUrlParser::chainEnded);
    connect(finalIntent, &Intent::cancelled, this, &IntentUrlParser::chainEnded);

    chain.first()->execute();
}

void IntentUrlParser::chainEnded()
{
    chainRunning = false;

    // Only a failed or cancelled chain restores the session the link migrated
    // away from; a successful one leaves the user where they are.
    if (pendingRestore && !currentChainSucceeded) {
        restorePreviousServer();
    }
    pendingRestore = false;

    startNextChain();

    // Only report the terminal state once the queue has fully drained, so a
    // queued follow-up link keeps the startup fallback out of the picture.
    if (!chainRunning && pendingChains.isEmpty()) {
        emit urlChainFinished(mainWindow->getRemoteClient()->getStatus() == StatusLoggedIn);
    }
}

void IntentUrlParser::restorePreviousServer()
{
    if (previousServerHost.isEmpty()) {
        return;
    }

    RemoteClient *client = mainWindow->getRemoteClient();
    const ClientStatus status = client->getStatus();

    // A failed/cancelled chain can fire while the client is still settling the
    // in-flight connection attempt (wrong password, connect timeout). Only
    // decide once the client has settled into logged-in or disconnected;
    // deciding mid-connect would strand the user offline from their previous
    // server.
    if (status == StatusDisconnected || status == StatusLoggedIn) {
        restoreToPreviousServer();
        return;
    }
    auto waitConnection = std::make_shared<QMetaObject::Connection>();
    *waitConnection = connect(client, &RemoteClient::statusChanged, this, [this, client, waitConnection]() {
        const ClientStatus settled = client->getStatus();
        if (settled == StatusDisconnected || settled == StatusLoggedIn) {
            QObject::disconnect(*waitConnection);
            restoreToPreviousServer();
        }
    });
}

void IntentUrlParser::restoreToPreviousServer()
{
    RemoteClient *client = mainWindow->getRemoteClient();

    // Back on the previous server already → nothing to undo.
    if (client->serverName().compare(previousServerHost, Qt::CaseInsensitive) == 0 &&
        QString::number(client->serverPort()) == previousServerPort) {
        return;
    }

    // When logged in somewhere, only intervene if that somewhere is the server
    // the link moved us to; if the user went elsewhere on their own, leave them.
    if (client->getStatus() == StatusLoggedIn) {
        const bool onMigrationTarget = client->serverName().compare(migrationTargetHost, Qt::CaseInsensitive) == 0 &&
                                       QString::number(client->serverPort()) == migrationTargetPort;
        if (!onMigrationTarget) {
            return;
        }

        ServersSettings &servers = SettingsCache::instance().servers();
        const int index = servers.findServerIndex(previousServerHost, previousServerPort);
        if (index >= 0 && servers.hasLoginData(previousServerHost, previousServerPort)) {
            const QString username =
                servers.getValue(QString("username%1").arg(index), "server", "server_details").toString();
            const QString password =
                servers.getValue(QString("password%1").arg(index), "server", "server_details").toString();
            client->connectToServer(previousServerHost, previousServerPort.toUInt(), username, password);
            return;
        }
        client->disconnectFromServer();
        return;
    }

    if (client->getStatus() != StatusDisconnected) {
        return;
    }

    // The link's connection attempt failed: reconnect to the previous server
    // when credentials are saved, otherwise stay offline.
    ServersSettings &servers = SettingsCache::instance().servers();
    const int index = servers.findServerIndex(previousServerHost, previousServerPort);
    if (index >= 0 && servers.hasLoginData(previousServerHost, previousServerPort)) {
        const QString username =
            servers.getValue(QString("username%1").arg(index), "server", "server_details").toString();
        const QString password =
            servers.getValue(QString("password%1").arg(index), "server", "server_details").toString();
        client->connectToServer(previousServerHost, previousServerPort.toUInt(), username, password);
    }
}
