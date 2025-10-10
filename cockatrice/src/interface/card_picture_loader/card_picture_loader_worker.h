/**
 * @file card_picture_loader_worker.h
 * @ingroup PictureLoader
 * @brief TODO: Document this.
 */

#ifndef PICTURE_LOADER_WORKER_H
#define PICTURE_LOADER_WORKER_H

#include "card_picture_loader_local.h"
#include "card_picture_loader_worker_work.h"
#include "card_picture_to_load.h"

#include <QLoggingCategory>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QObject>
#include <QQueue>
#include <QTimer>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database.h>

#define REDIRECT_HEADER_NAME "redirects"
#define REDIRECT_ORIGINAL_URL "original"
#define REDIRECT_URL "redirect"
#define REDIRECT_TIMESTAMP "timestamp"
#define REDIRECT_CACHE_FILENAME "cache.ini"

inline Q_LOGGING_CATEGORY(CardPictureLoaderWorkerLog, "card_picture_loader.worker");

class CardPictureLoaderWorkerWork;
class CardPictureLoaderWorker : public QObject
{
    Q_OBJECT
public:
    explicit CardPictureLoaderWorker();
    ~CardPictureLoaderWorker() override;

    void enqueueImageLoad(const ExactCard &card); // Starts a thread for the image to be loaded
    void queueRequest(const QUrl &url, CardPictureLoaderWorkerWork *worker); // Queues network requests for load threads
    void clearNetworkCache();

public slots:
    QNetworkReply *makeRequest(const QUrl &url, CardPictureLoaderWorkerWork *workThread);
    void processQueuedRequests();
    bool processSingleRequest();
    void handleImageLoaded(const ExactCard &card, const QImage &image);
    void cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl);
    void removedCachedUrl(const QUrl &url);

private:
    QThread *pictureLoaderThread;
    QNetworkAccessManager *networkManager;
    QNetworkDiskCache *cache;
    QHash<QUrl, QPair<QUrl, QDateTime>> redirectCache; // Stores redirect and timestamp
    QString cacheFilePath;                             // Path to persistent storage
    static constexpr int CacheTTLInDays = 30;          // TODO: Make user configurable
    bool picDownload;
    QQueue<QPair<QUrl, CardPictureLoaderWorkerWork *>> requestLoadQueue;

    int requestQuota;
    QTimer requestTimer; // Timer for refreshing request quota

    CardPictureLoaderLocal *localLoader;
    QSet<QString> currentlyLoading; // for deduplication purposes. Contains pixmapCacheKey

    QUrl getCachedRedirect(const QUrl &originalUrl) const;
    void loadRedirectCache();
    void saveRedirectCache() const;
    void cleanStaleEntries();

private slots:
    void resetRequestQuota();
    void handleImageLoadEnqueued(const ExactCard &card);

signals:
    void imageLoadEnqueued(const ExactCard &card);
    void imageLoaded(const ExactCard &card, const QImage &image);
    void imageRequestQueued(const QUrl &url, const ExactCard &card, const QString &setName);
    void imageRequestSucceeded(const QUrl &url);
};

#endif // PICTURE_LOADER_WORKER_H
