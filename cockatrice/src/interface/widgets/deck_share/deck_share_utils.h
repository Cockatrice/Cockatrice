/**
 * @file deck_share_utils.h
 * @ingroup DeckShareWidgets
 */
//! \todo Document this file.

#ifndef DECK_SHARE_UTILS_H
#define DECK_SHARE_UTILS_H

#include <QDateTime>
#include <QString>

class AbstractClient;

/**
 * @brief Shared helpers for creating temporary deck shares.
 */
namespace DeckShareUtils
{

/**
 * @brief Builds the cockatrice:// link for a freshly created deck share.
 * @param client Used to embed the target server's hostname and port.
 * @param token The share token from Response_DeckShareCreate.
 */
QString buildShareLink(const AbstractClient *client, const QString &token);

/**
 * @brief Copies the share link to the clipboard.
 * @return The link that was copied.
 */
QString copyShareLinkToClipboard(const QString &link);

/**
 * @brief Formats the expiration timestamp for a share.
 */
QString formatShareExpiry(const QDateTime &expiry);

} // namespace DeckShareUtils

#endif // DECK_SHARE_UTILS_H