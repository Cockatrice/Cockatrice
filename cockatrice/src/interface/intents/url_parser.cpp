#include "url_parser.h"

#include "../window_main.h"
#include "contexts/context_join_game.h"
#include "intent_join_server_game.h"
#include "intent_login.h"

#include <QDebug>
#include <QMessageBox>
#include <QUrl>
#include <QUrlQuery>
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
