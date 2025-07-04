#ifndef PICTURE_LOADER_WORKER_H
#define PICTURE_LOADER_WORKER_H

#include "../../../game/cards/card_database.h"
#include "../../../game/cards/card_info.h"
#include "picture_loader_local.h"
#include "picture_loader_worker_work.h"
#include "picture_to_load.h"

#include <QLoggingCategory>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QObject>
#include <QQueue>
#include <QTimer>

#define REDIRECT_HEADER_NAME "redirects"
#define REDIRECT_ORIGINAL_URL "original"
#define REDIRECT_URL "redirect"
#define REDIRECT_TIMESTAMP "timestamp"
#define REDIRECT_CACHE_FILENAME "cache.ini"

inline Q_LOGGING_CATEGORY(PictureLoaderWorkerLog, "picture_loader.worker");

class PictureLoaderWorkerWork;
class PictureLoaderWorker : public QObject
{
    Q_OBJECT
public:
    explicit PictureLoaderWorker();
    ~PictureLoaderWorker() override;

    void enqueueImageLoad(const CardInfoPtr &card);                      // Starts a thread for the image to be loaded
    void queueRequest(const QUrl &url, PictureLoaderWorkerWork *worker); // Queues network requests for load threads
    void clearNetworkCache();

public slots:
    QNetworkReply *makeRequest(const QUrl &url, PictureLoaderWorkerWork *workThread);
    void processQueuedRequests();
    void processSingleRequest();
    void imageLoadedSuccessfully(const CardInfoPtr &card, const QImage &image);
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
    QQueue<QPair<QUrl, PictureLoaderWorkerWork *>> requestLoadQueue;
    QTimer requestTimer; // Timer for processing delayed requests

    PictureLoaderLocal *localLoader;
    QSet<CardInfoPtr> currentlyLoading; // for deduplication purposes

    QUrl getCachedRedirect(const QUrl &originalUrl) const;
    void loadRedirectCache();
    void saveRedirectCache() const;
    void cleanStaleEntries();

private slots:
    void handleImageLoadEnqueued(const CardInfoPtr &card);

signals:
    void imageLoadEnqueued(const CardInfoPtr &card);
    void imageLoaded(CardInfoPtr card, const QImage &image);
    void imageLoadQueued(const QUrl &url, PictureLoaderWorkerWork *worker);
    void imageLoadSuccessful(const QUrl &url, PictureLoaderWorkerWork *worker);
};

#endif // PICTURE_LOADER_WORKER_H
