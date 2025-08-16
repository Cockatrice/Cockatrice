#ifndef PICTURE_LOADER_WORKER_WORK_H
#define PICTURE_LOADER_WORKER_WORK_H

#include "../../../game/cards/card_database.h"
#include "picture_loader_worker.h"
#include "picture_to_load.h"

#include <QLoggingCategory>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QThread>

#define REDIRECT_HEADER_NAME "redirects"
#define REDIRECT_ORIGINAL_URL "original"
#define REDIRECT_URL "redirect"
#define REDIRECT_TIMESTAMP "timestamp"
#define REDIRECT_CACHE_FILENAME "cache.ini"

inline Q_LOGGING_CATEGORY(PictureLoaderWorkerWorkLog, "picture_loader.worker");

class PictureLoaderWorker;

class PictureLoaderWorkerWork : public QObject
{
    Q_OBJECT
public:
    explicit PictureLoaderWorkerWork(const PictureLoaderWorker *worker, const ExactCard &toLoad);

    PictureToLoad cardToDownload;

public slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    bool picDownload;

    void startNextPicDownload();
    void picDownloadFailed();
    void handleFailedReply(const QNetworkReply *reply);
    void handleSuccessfulReply(QNetworkReply *reply);
    QImage tryLoadImageFromReply(QNetworkReply *reply);
    void concludeImageLoad(const QImage &image);

private slots:
    void picDownloadChanged();

signals:
    /**
     * Emitted when this worker has successfully loaded the image or has exhausted all attempts at loading the image.
     * Failures are represented by an empty QImage.
     * Note that this object will delete itself as this signal is emitted.
     */
    void imageLoaded(const ExactCard &card, const QImage &image);

    /**
     * Emitted when a request did not return a 400 or 500 response
     */
    void requestSucceeded(const QUrl &url);
    void requestImageDownload(const QUrl &url, PictureLoaderWorkerWork *instance);

    void urlRedirected(const QUrl &originalUrl, const QUrl &redirectUrl);
    void cachedUrlInvalidated(const QUrl &url);
};

#endif // PICTURE_LOADER_WORKER_WORK_H
