#ifndef COCKATRICE_USER_AVATAR_PROVIDER_H
#define COCKATRICE_USER_AVATAR_PROVIDER_H

#include <QCache>
#include <QElapsedTimer>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QSet>
#include <QString>
#include <qtmetamacros.h>

class AbstractClient;

class UserAvatarProvider : public QObject
{
    Q_OBJECT

public:
    explicit UserAvatarProvider(AbstractClient *client, QObject *parent = nullptr);

    void requestAvatar(const QString &userName);
    const QMap<QString, QPixmap> &cache() const;

signals:
    void avatarUpdated(const QString &userName);

private:
    static constexpr qint64 RetryDelayMs = 5000;
    AbstractClient *client;
    QMap<QString, QPixmap> avatarCache;
    QSet<QString> pending;
    // Keep transient failures retryable without re-requesting on every scroll.
    QElapsedTimer retryClock;
    QCache<QString, qint64> retryAfter{512};
};

#endif // COCKATRICE_USER_AVATAR_PROVIDER_H
