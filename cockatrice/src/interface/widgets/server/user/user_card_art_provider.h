#ifndef COCKATRICE_USER_CARD_ART_PROVIDER_H
#define COCKATRICE_USER_CARD_ART_PROVIDER_H

#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QQueue>
#include <QSet>
#include <libcockatrice/card/printing/exact_card.h>

class UserCardArtProvider : public QObject
{
    Q_OBJECT

public:
    explicit UserCardArtProvider(QObject *parent = nullptr);

    void requestCardArt(const QString &userName, const QString &cardName, const QString &providerId);
    const QMap<QString, QPixmap> &cache() const;
    static QPixmap cropCardArt(const QPixmap &fullRes, const ExactCard &card);

signals:
    void cardArtUpdated(const QString &userName);

public slots:
    void onDatabaseReady();

private:
    bool dbReady = false;
    static constexpr int MaxCacheEntries = 1024;
    QList<QString> cacheInsertionOrder; // FIFO eviction
    QMap<QString, QPixmap> cardArtCache;
    QSet<QString> pending;
    QQueue<QString> queue;

    void processQueue();
    void insertIntoCache(const QString &key, const QPixmap &pixmap);
};

#endif // COCKATRICE_USER_CARD_ART_PROVIDER_H