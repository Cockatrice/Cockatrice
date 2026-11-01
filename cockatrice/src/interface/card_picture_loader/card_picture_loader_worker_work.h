#ifndef PICTURE_LOADER_WORKER_WORK_H
#define PICTURE_LOADER_WORKER_WORK_H

#include "card_picture_to_load.h"

#include <QImage>
#include <QLoggingCategory>
#include <QObject>
#include <QString>
#include <QSet>
#include <QTimer>
#include <QUrl>
#include <qtmetamacros.h>

class ExactCard;
class QNetworkReply;
class QUrl;
class ServerRateLimiter;

inline Q_LOGGING_CATEGORY(CardPictureLoaderWorkerWorkLog, "card_picture_loader.worker");

class CardPictureLoaderWorker;

/**
 * @class CardPictureLoaderWorkerWork
 * @ingroup PictureLoader
 * @brief Handles downloading a single card image from network or local sources.
 *
 * Responsibilities:
 * - Try to load images from all possible URLs and sets for a given ExactCard.
 * - Handle network redirects and cache invalidation.
 * - Check for blacklisted images and discard them.
 * - Emit signals when image is successfully loaded or all attempts fail.
 * - Delete itself after loading completes.
 */
class CardPictureLoaderWorkerWork : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a worker for downloading a specific card image.
     * @param worker The orchestrating CardPictureLoaderWorker; the work object becomes its child so
     *               it is destroyed with the worker even if it never reaches concludeImageLoad().
     * @param toLoad The ExactCard to download
     */
    explicit CardPictureLoaderWorkerWork(CardPictureLoaderWorker *worker, const ExactCard &toLoad);

    CardPictureToLoad cardToDownload; ///< The card and associated URLs to try downloading

    /** @brief Shared per-server 429 backoff state. */
    static const ServerRateLimiter &rateLimiter();

    /** @brief A corrupt cached response gets one network retry per URL and work item. */
    [[nodiscard]] bool shouldBypassCache(const QUrl &url) const;

    /**
     * @brief Starts downloading the next URL for this card.
     *
     * Skips URLs whose server is currently in 429 backoff, either waiting the
     * backoff out or falling through to the other configured sources.
     */
    void startNextPicDownload();

    /**
     * @brief Schedules a deferred retry after the relevant server backoff expires.
     * @param preferredHost The server that is actually blocking the request, or an empty
     *                      string to use the current URL's server
     *
     * Waits on the blocking server's backoff deadline, otherwise on the earliest active
     * backoff. If no servers are in backoff, concludes with failure. Otherwise resets the
     * CardPictureToLoad indices and retries after the backoff period.
     */
    void scheduleDeferredRetry(const QString &preferredHost = {});

public slots:
    /**
     * @brief Handles a finished network reply for the card image.
     * @param reply The QNetworkReply object to process. Ownership is transferred.
     */
    void handleNetworkReply(QNetworkReply *reply);

private:
    bool picDownload; ///< Whether network downloading is enabled
    QTimer *deferredRetry = nullptr; ///< Owned retry, cancellable when downloads are disabled
    QSet<QUrl> retriedCacheUrls;

    bool retryCachedUrl(const QUrl &url);
    QSet<QUrl> redirectVisited; ///< Redirect chain for the current source URL

    static ServerRateLimiter s_rateLimiter; ///< Shared per-server 429 backoff state

    /** @brief Called when all URLs have been exhausted or download failed. */
    void picDownloadFailed();

    /** @brief Processes a failed network reply. */
    void handleFailedReply(const QNetworkReply *reply);

    /** @brief Processes a successful network reply. */
    void handleSuccessfulReply(QNetworkReply *reply);

    /**
     * @brief Finalizes the image loading process.
     * @param image The loaded image (empty if failed)
     *
     * Emits imageLoaded() and deletes this object.
     */
    void concludeImageLoad(const QImage &image);

private slots:
    /** @brief Updates the picDownload setting when it changes. */
    void picDownloadChanged();

signals:
    /**
     * @brief Emitted when the image has been loaded or all attempts failed.
     * @param card The card corresponding to the image
     * @param image The loaded image (empty if failed)
     *
     * The worker deletes itself after emitting this signal.
     */
    void imageLoaded(const ExactCard &card, const QImage &image);

    /** @brief Request that a URL be downloaded. */
    void requestImageDownload(const QUrl &url, CardPictureLoaderWorkerWork *instance);

    /** @brief Emitted when a URL has been redirected. */
    void urlRedirected(const QUrl &originalUrl, const QUrl &redirectUrl);

    /** @brief A redirect mapping is unsafe and should be removed. */
    void redirectInvalidated(const QUrl &url);

    /** @brief Emitted when a server returned HTTP 429. */
    void rateLimited(const QString &host);

    /** @brief Emitted when a cached URL is invalid and must be removed. */
    void cachedUrlInvalidated(const QUrl &url);
};

#endif // PICTURE_LOADER_WORKER_WORK_H
