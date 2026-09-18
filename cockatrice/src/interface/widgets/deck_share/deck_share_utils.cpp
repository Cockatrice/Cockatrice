#include "deck_share_utils.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QLocale>
#include <QTimeZone>
#include <QUrl>
#include <QUrlQuery>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_share_create.pb.h>

namespace DeckShareUtils
{

QString buildShareLink(const AbstractClient *client, const QString &token)
{
    QUrl url;
    url.setScheme(QStringLiteral("cockatrice"));
    url.setHost(QStringLiteral("opendeck"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("share"), token);
    query.addQueryItem(QStringLiteral("hostname"), client->serverName());
    query.addQueryItem(QStringLiteral("port"), QString::number(client->serverPort()));
    url.setQuery(query);

    return url.toString(QUrl::FullyEncoded);
}

QString copyShareLinkToClipboard(const QString &link)
{
    QGuiApplication::clipboard()->setText(link);
    return link;
}

QString formatShareExpiry(const QDateTime &expiry)
{
    return QLocale().toString(expiry.toLocalTime(), QLocale::ShortFormat);
}

ShareResponse handleShareResponse(const AbstractClient *client, const Response &response)
{
    const Response_DeckShareCreate &resp = response.GetExtension(Response_DeckShareCreate::ext);
    const QString token = QString::fromStdString(resp.token());

    const QString link = buildShareLink(client, token);
    copyShareLinkToClipboard(link);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    const QDateTime expiry = QDateTime::fromSecsSinceEpoch(resp.expires_at(), QTimeZone::UTC);
#else
    const QDateTime expiry = QDateTime::fromSecsSinceEpoch(resp.expires_at(), Qt::UTC);
#endif

    return {link, expiry};
}

} // namespace DeckShareUtils