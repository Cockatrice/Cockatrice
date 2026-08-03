#include "url_parser.h"

#include "../window_main.h"
#include "contexts/context_join_game.h"
#include "intent_join_server_game.h"
#include "intent_login.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

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
    auto ctx = new ContextJoinGame();

    ctx->roomContext.serverContext.hostname = query.queryItemValue("hostname");
    ctx->roomContext.serverContext.port = query.queryItemValue("port");

    bool ok = false;
    ctx->roomContext.roomId = query.queryItemValue("roomid").toInt(&ok);

    if (!ok) {
        qWarning() << "Invalid or missing roomId";
        delete ctx;
        return;
    }

    ok = false;
    ctx->gameId = query.queryItemValue("gameid").toInt(&ok);

    if (!ok) {
        qWarning() << "Invalid or missing gameId";
        delete ctx;
        return;
    }

    // The join game intent owns the context and the credential lookup; once the
    // chain finishes (or fails) it deletes the whole tree.
    auto joinGameIntent = new IntentJoinServerGame(mainWindow->getTabSupervisor(), mainWindow->getRemoteClient(), ctx);
    joinGameIntent->setParent(this);

    auto getLoginCredentialsIntent = new IntentGetLoginCredentials(&ctx->roomContext.serverContext);
    getLoginCredentialsIntent->setParent(joinGameIntent);

    connect(getLoginCredentialsIntent, &Intent::finished, joinGameIntent, &Intent::execute);
    connect(getLoginCredentialsIntent, &Intent::failed, joinGameIntent, &Intent::failed);

    getLoginCredentialsIntent->execute();
}
