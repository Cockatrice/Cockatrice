#include "url_parser.h"

#include "contexts/context_connect_to_server.h"
#include "contexts/context_join_game.h"
#include "contexts/context_join_room.h"
#include "intent_connect_to_server.h"
#include "intent_join_server_game.h"
#include "intent_join_server_room.h"
#include "intent_login.h"

#include <QLoggingCategory>
#include <libcockatrice/utility/url_utils.h>

Q_LOGGING_CATEGORY(UrlParserLog, "url_parser")

Intent *UrlParser::parse(const QString &url,
                         ConnectionController *controller,
                         TabSupervisor *supervisor,
                         QWidget *dialogParent,
                         QObject *parent)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(url);
    if (!parsed) {
        qCWarning(UrlParserLog) << "Could not parse cockatrice:// URL:" << url;
        return nullptr;
    }

    qCDebug(UrlParserLog) << "Parsed cockatrice://joingame" << "host=" << parsed->hostname << "port=" << parsed->port
                          << "room=" << parsed->roomId << "game=" << parsed->gameId << "spectate=" << parsed->spectator;

    // Build the intent chain.  Each intent is parented to the root so that the
    // whole chain is cleaned up when the root is deleted.
    ContextConnectToServer connectCtx{parsed->hostname, parsed->port};
    auto *intentConnect = new IntentConnectToServer(connectCtx, controller, dialogParent, parent);
    auto *intentLogin = new IntentLogin(controller, parent);

    ContextJoinRoom joinRoomCtx{parsed->roomId};
    auto *intentJoinRoom = new IntentJoinServerRoom(joinRoomCtx, supervisor, controller, parent);

    ContextJoinGame joinGameCtx{parsed->gameId, parsed->roomId, parsed->spectator};
    auto *intentJoinGame = new IntentJoinServerGame(joinGameCtx, supervisor, controller, parent);

    // Chain: connect → login → joinRoom → joinGame
    QObject::connect(intentConnect, &Intent::finished, intentLogin, [intentLogin](bool ok) {
        if (ok) {
            intentLogin->execute();
        } else {
            qCWarning(UrlParserLog) << "Connect step failed — aborting intent chain";
            intentLogin->abort();
        }
    });

    QObject::connect(intentLogin, &Intent::finished, intentJoinRoom, [intentJoinRoom](bool ok) {
        if (ok) {
            intentJoinRoom->execute();
        } else {
            qCWarning(UrlParserLog) << "Login step failed — aborting intent chain";
            intentJoinRoom->abort();
        }
    });

    QObject::connect(intentJoinRoom, &Intent::finished, intentJoinGame, [intentJoinGame](bool ok) {
        if (ok) {
            intentJoinGame->execute();
        } else {
            qCWarning(UrlParserLog) << "Join-room step failed — aborting intent chain";
            intentJoinGame->abort();
        }
    });

    return intentConnect;
}
