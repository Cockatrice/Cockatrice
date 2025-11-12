#ifndef PICTURE_LOADER_WORKER_WORK_H
#define PICTURE_LOADER_WORKER_WORK_H

#include "card_picture_loader_worker.h"
#include "card_picture_to_load.h"

#include <QLoggingCategory>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QThread>
#include <libcockatrice/card/database/card_database.h>

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
     * @param worker The orchestrating CardPictureLoaderWorker
     * @param toLoad The ExactCard to download
     */
    explicit CardPictureLoaderWorkerWork(const CardPictureLoaderWorker *worker, const ExactCard &toLoad);

    CardPictureToLoad cardToDownload; ///< The card and associated URLs to try downloading

public slots:
    /**
     * @brief Handles a finished network reply for the card image.
     * @param reply The QNetworkReply object to process. Ownership is transferred.
     */
    void handleNetworkReply(QNetworkReply *reply);

private:
    bool picDownload; ///< Whether network downloading is enabled

    /** @brief Starts downloading the next URL for this card. */
    void startNextPicDownload();

    /** @brief Called when all URLs have been exhausted or download failed. */
    void picDownloadFailed();

    /** @brief Processes a failed network reply. */
    void handleFailedReply(const QNetworkReply *reply);

    /** @brief Processes a successful network reply. */
    void handleSuccessfulReply(QNetworkReply *reply);

    /**
     * @brief Attempts to read an image from a network reply.
     * @param reply The network reply
     * @return The loaded QImage or an empty image if reading failed
     */
    QImage tryLoadImageFromReply(QNetworkReply *reply);

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

    /** @brief Emitted when a network request completes successfully. */
    void requestSucceeded(const QUrl &url);

    /** @brief Request that a URL be downloaded. */
    void requestImageDownload(const QUrl &url, CardPictureLoaderWorkerWork *instance);

    /** @brief Emitted when a URL has been redirected. */
    void urlRedirected(const QUrl &originalUrl, const QUrl &redirectUrl);

    /** @brief Emitted when a cached URL is invalid and must be removed. */
    void cachedUrlInvalidated(const QUrl &url);
};

#endif // PICTURE_LOADER_WORKER_WORK_H
