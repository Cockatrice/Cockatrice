#include "deck_share_utils.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QTimeZone>
#include <libcockatrice/network/client/abstract/abstract_client.h>

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

} // namespace DeckShareUtils