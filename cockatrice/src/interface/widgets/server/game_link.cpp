#include "game_link.h"

#include <QUrl>
#include <QUrlQuery>

QString makeGameJoinLink(const QString &hostname, int port, int roomId, int gameId, const QString &description)
{
    QUrl url;
    url.setScheme("cockatrice");
    url.setHost("joingame");
    QUrlQuery query;
    query.addQueryItem("hostname", hostname);
    query.addQueryItem("port", QString::number(port));
    query.addQueryItem("roomid", QString::number(roomId));
    query.addQueryItem("gameid", QString::number(gameId));
    if (!description.isEmpty()) {
        // addQueryItem percent-encodes, so arbitrary descriptions (quotes,
        // ampersands, non-ASCII…) survive the trip through chat.
        query.addQueryItem("game", description);
    }
    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}
