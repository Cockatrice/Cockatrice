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

inline Q_LOGGING_CATEGORY(CardPictureLoaderWorkerLog, "card_picture_loader.worker")

    class CardPictureLoaderWorkerWork;

/**
 * @class CardPictureLoaderWorker
 * @ingroup PictureLoader
 * @brief Handles asynchronous loading of card images, both locally and via network.
 *
 * Responsibilities:
 * - Maintain a queue of network image requests with rate-limiting.
 * - Load images from local cache first via CardPictureLoaderLocal.
 * - Handle network redirects and persistent caching of redirects.
 * - Deduplicate simultaneous requests for the same card.
 * - Emit signals for status updates and loaded images.
 */
class CardPictureLoaderWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a CardPictureLoaderWorker.
     *
     * Initializes network manager, cache, redirect cache, local loader, and request quota timer.
     */
    explicit CardPictureLoaderWorker();

    ~CardPictureLoaderWorker() override;

    /**
     * @brief Enqueues an ExactCard for loading.
     * @param card ExactCard to load
     *
     * This will first try to load the image locally; if that fails, it will enqueue a network request.
     */
    void enqueueImageLoad(const ExactCard &card);

    /**
     * @brief Queues a network request for a given URL and worker thread.
     * @param url URL to load
     * @param worker Worker handling this request
     */
    void queueRequest(const QUrl &url, CardPictureLoaderWorkerWork *worker);

    /** @brief Clears the network cache and redirect cache. */
    void clearNetworkCache();

public slots:
    /**
     * @brief Makes a network request for the given URL using the specified worker.
     * @param url URL to load
     * @param workThread Worker handling the request
     * @return The QNetworkReply object representing the request
     */
    QNetworkReply *makeRequest(const QUrl &url, CardPictureLoaderWorkerWork *workThread);

    /** @brief Processes all queued requests respecting the request quota. */
    void processQueuedRequests();

    /**
     * @brief Processes a single queued request.
     * @return true if a request was processed, false if queue is empty.
     */
    bool processSingleRequest();

    /**
     * @brief Handles an image that has finished loading.
     * @param card The ExactCard that was loaded
     * @param image The loaded QImage; empty if loading failed
     */
    void handleImageLoaded(const ExactCard &card, const QImage &image);

    /** @brief Caches a redirect mapping between original and redirected URL. */
    void cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl);

    /** @brief Removes a URL from the network cache. */
    void removedCachedUrl(const QUrl &url);

private:
    QThread *pictureLoaderThread;                      ///< Thread for executing worker tasks
    QNetworkAccessManager *networkManager;             ///< Network manager for HTTP requests
    QNetworkDiskCache *cache;                          ///< Disk cache for downloaded images
    QHash<QUrl, QPair<QUrl, QDateTime>> redirectCache; ///< Maps original URLs to redirects with timestamp
    QString cacheFilePath;                             ///< Path to persistent redirect cache file
    static constexpr int CacheTTLInDays = 30;          ///< Time-to-live for redirect cache entries (days)
    bool picDownload;                                  ///< Whether downloading images from network is enabled
    QQueue<QPair<QUrl, CardPictureLoaderWorkerWork *>> requestLoadQueue; ///< Queue of pending network requests

    int requestQuota;    ///< Remaining requests allowed per second
    QTimer requestTimer; ///< Timer to reset the request quota

    CardPictureLoaderLocal *localLoader; ///< Loader for local images
    QSet<QString> currentlyLoading;      ///< Deduplication: contains pixmapCacheKey currently being loaded

    /** @brief Returns cached redirect URL for the given original URL, if available. */
    [[nodiscard]] QUrl getCachedRedirect(const QUrl &originalUrl) const;

    /** @brief Loads redirect cache from disk. */
    void loadRedirectCache();

    /** @brief Saves redirect cache to disk. */
    void saveRedirectCache() const;

    /** @brief Removes stale redirect entries older than TTL. */
    void cleanStaleEntries();

private slots:
    /** @brief Resets the request quota for rate-limiting. */
    void resetRequestQuota();

    /** @brief Handles image load requests enqueued on this worker. */
    void handleImageLoadEnqueued(const ExactCard &card);

signals:
    /** @brief Emitted when an image load is enqueued. */
    void imageLoadEnqueued(const ExactCard &card);

    /** @brief Emitted when an image has finished loading. */
    void imageLoaded(const ExactCard &card, const QImage &image);

    /** @brief Emitted when a request is added to the network queue. */
    void imageRequestQueued(const QUrl &url, const ExactCard &card, const QString &setName);

    /** @brief Emitted when a network request successfully completes. */
    void imageRequestSucceeded(const QUrl &url);
};

#endif // PICTURE_LOADER_WORKER_H
