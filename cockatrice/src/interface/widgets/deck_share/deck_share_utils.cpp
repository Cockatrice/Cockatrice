#include "deck_share_utils.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QTimeZone>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_share_create.pb.h>

namespace DeckShareUtils
{

QString buildShareLink(const AbstractClient *client, const QString &token)
{
    return QString("cockatrice://opendeck?share=%1&hostname=%2&port=%3")
        .arg(token, client->serverName(), QString::number(client->serverPort()));
}

QString copyShareLinkToClipboard(const QString &link)
{
    QGuiApplication::clipboard()->setText(link);
    return link;
}

QString formatShareExpiry(const QDateTime &expiry)
{
    return expiry.toLocalTime().toString();
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