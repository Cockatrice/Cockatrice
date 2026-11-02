#ifndef COCKATRICE_USER_CARD_ART_PROVIDER_H
#define COCKATRICE_USER_CARD_ART_PROVIDER_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QQueue>
#include <QString>
#include <qtmetamacros.h>

class ExactCard;

class UserCardArtProvider : public QObject
{
    Q_OBJECT

public:
    explicit UserCardArtProvider(QObject *parent = nullptr);

    void requestCardArt(const QString &userName, const QString &cardName, const QString &providerId);
    const QMap<QString, QPixmap> &cache() const;
    static QString cacheKey(const QString &userName, const QString &cardName, const QString &providerId);
    static QPixmap cropCardArt(const QPixmap &fullRes, const ExactCard &card);

signals:
    void cardArtUpdated(const QString &userName);

public slots:
    void onDatabaseReady();

private:
    struct Request
    {
        QString userName;
        QString cardName;
        QString providerId;
    };

    bool dbReady = false;
    bool processingQueue = false;
    static constexpr int MaxCacheEntries = 1024;
    QList<QString> cacheInsertionOrder; // FIFO eviction
    QMap<QString, QPixmap> cardArtCache;
    // Retain source identities for pending work and bounded cached entries, so a
    // database replacement can resolve fresh CardInfo objects for every banner.
    QMap<QString, Request> requests;
    QMap<QString, QString> currentKeys;
    QMap<QString, QObject *> activeRequests;
    QQueue<QString> queue;

    void processQueue();
    void forgetRequest(const QString &key);
    void finishRequest(const QString &key, QObject *context, const QPixmap &pixmap);
    void insertIntoCache(const QString &key, const QPixmap &pixmap);
};

#endif // COCKATRICE_USER_CARD_ART_PROVIDER_H