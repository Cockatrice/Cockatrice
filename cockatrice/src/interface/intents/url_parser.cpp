#include "url_parser.h"

#include "../window_main.h"
#include "contexts/context_join_room.h"
#include "intent_join_server_game.h"
#include "intent_join_server_room.h"
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

    if (url.scheme() != "cockatrice")
        return;

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
        return;
    }

    ok = false;
    ctx->gameId = query.queryItemValue("gameid").toInt(&ok);

    if (!ok) {
        qWarning() << "Invalid or missing gameId";
        return;
    }

    auto getLoginCredentialsIntent =
        new IntentGetLoginCredentials(mainWindow->getRemoteClient(), &ctx->roomContext.serverContext);

    auto joinGameIntent = new IntentJoinServerGame(mainWindow->getTabSupervisor(), mainWindow->getRemoteClient(), ctx);

    connect(getLoginCredentialsIntent, &Intent::finished, joinGameIntent, &Intent::execute);

    getLoginCredentialsIntent->execute();
}
