#include "deck_sync_profile.h"

#include <QCryptographicHash>

DeckSyncProfile::DeckSyncProfile(const QString &_host, int _port, const QString &_username)
    : host(_host), port(_port), username(_username)
{
}

bool DeckSyncProfile::isValid() const
{
    return !host.isEmpty() && port > 0 && !username.isEmpty();
}

QString DeckSyncProfile::key() const
{
    if (!isValid()) {
        return {};
    }
    // NUL separators keep the triple injective: neither IPv6 hosts nor
    // usernames containing ':' can collide two profiles into one file.
    // Lowercasing is safe on both parts: DNS names are case-insensitive, and
    // Servatrice usernames are unique under a case-insensitive collation
    // (cockatrice_users uses utf8mb4_unicode_ci), so both spellings are one account.
    QByteArray blob = host.trimmed().toLower().toUtf8();
    blob.append('\0');
    blob.append(QByteArray::number(port));
    blob.append('\0');
    blob.append(username.trimmed().toLower().toUtf8());
    return QCryptographicHash::hash(blob, QCryptographicHash::Sha1).toHex();
}
