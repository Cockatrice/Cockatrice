#ifndef LIBCOCKATRICE_URL_UTILS_H
#define LIBCOCKATRICE_URL_UTILS_H

#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <optional>

namespace UrlUtils
{

/**
 * @brief Scans @p args and returns the first entry that starts with
 *        @p schemePrefix (case-insensitive, per RFC 3986), or an empty string
 *        if none is found.  Only the first match is returned; subsequent
 *        matching args are ignored.
 *
 * Use this to extract a custom-scheme URL from QCommandLineParser positional
 * arguments or raw argv arrays.
 */
inline QString findUrlArgument(const QStringList &args, const QString &schemePrefix)
{
    for (const QString &arg : args) {
        if (arg.startsWith(schemePrefix, Qt::CaseInsensitive))
            return arg;
    }
    return {};
}

/**
 * @brief Parsed shape of a cockatrice-oracle:// URL.
 *
 * Currently only @c update is recognised; other hosts are ignored.
 */
struct OracleUrlAction
{
    bool isUpdate{false};
    bool spoilersOnly{false};
};

/**
 * @brief Parse a cockatrice-oracle:// URL into an OracleUrlAction.
 *
 * Recognised forms:
 *   cockatrice-oracle://update
 *   cockatrice-oracle://update?spoilers=1
 *
 * Returns a default-constructed action (@c isUpdate == false) for any URL
 * whose host is not @c update.  Host matching is case-insensitive.
 */
inline OracleUrlAction parseOracleUrl(const QString &url)
{
    OracleUrlAction action;
    const QUrl parsed(url);
    if (parsed.host().toLower() != QStringLiteral("update"))
        return action;
    action.isUpdate = true;
    action.spoilersOnly = QUrlQuery(parsed.query()).queryItemValue(QStringLiteral("spoilers")) == QStringLiteral("1");
    return action;
}

/**
 * @brief Parsed parameters from a cockatrice://joingame URL.
 */
struct JoinGameUrlParams
{
    QString hostname;
    quint16 port;
    int roomId;
    int gameId;
    bool spectator;
};

/**
 * @brief Parse a cockatrice://joingame URL into its parameters.
 *
 * Recognised forms:
 *   cockatrice://joingame?hostname=H&port=P&roomid=R&gameid=G
 *   cockatrice://joingame?hostname=H&port=P&roomid=R&gameid=G&spectate=1
 *
 * Validation:
 *   - scheme must be "cockatrice" (case-insensitive)
 *   - host must be "joingame" (case-insensitive)
 *   - hostname query param required
 *   - port required, 1..65535
 *   - roomid required, >= 0
 *   - gameid required, >= 0
 *   - spectate=1 sets spectator true; any other value (including absence) is false
 *
 * Credentials in the query (username/password) are intentionally ignored.
 *
 * @return std::nullopt for unrecognised or malformed URLs.
 */
inline std::optional<JoinGameUrlParams> parseJoinGameUrl(const QString &url)
{
    const QUrl parsed(url);
    if (!parsed.isValid())
        return std::nullopt;
    if (parsed.scheme().toLower() != QStringLiteral("cockatrice"))
        return std::nullopt;
    if (parsed.host().toLower() != QStringLiteral("joingame"))
        return std::nullopt;

    const QUrlQuery query(parsed.query());

    const QString hostname = query.queryItemValue(QStringLiteral("hostname"));
    if (hostname.isEmpty())
        return std::nullopt;

    bool portOk = false;
    const uint portVal = query.queryItemValue(QStringLiteral("port")).toUInt(&portOk);
    if (!portOk || portVal == 0 || portVal > 65535)
        return std::nullopt;

    bool roomOk = false;
    const int roomId = query.queryItemValue(QStringLiteral("roomid")).toInt(&roomOk);
    if (!roomOk || roomId < 0)
        return std::nullopt;

    bool gameOk = false;
    const int gameId = query.queryItemValue(QStringLiteral("gameid")).toInt(&gameOk);
    if (!gameOk || gameId < 0)
        return std::nullopt;

    JoinGameUrlParams params;
    params.hostname = hostname;
    params.port = static_cast<quint16>(portVal);
    params.roomId = roomId;
    params.gameId = gameId;
    params.spectator = query.queryItemValue(QStringLiteral("spectate")) == QStringLiteral("1");
    return params;
}

} // namespace UrlUtils

#endif // LIBCOCKATRICE_URL_UTILS_H
