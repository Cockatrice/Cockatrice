#ifndef COCKATRICE_USER_AVATAR_PROVIDER_H
#define COCKATRICE_USER_AVATAR_PROVIDER_H

#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QSet>

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
    AbstractClient *client;
    QMap<QString, QPixmap> avatarCache;
    QSet<QString> pending;
};

#endif // COCKATRICE_USER_AVATAR_PROVIDER_H