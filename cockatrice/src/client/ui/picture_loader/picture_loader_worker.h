#ifndef PICTURE_LOADER_WORKER_H
#define PICTURE_LOADER_WORKER_H

#include "../../../game/cards/card_database.h"
#include "picture_loader_worker_work.h"
#include "picture_to_load.h"

#include <QLoggingCategory>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>

#define REDIRECT_HEADER_NAME "redirects"
#define REDIRECT_ORIGINAL_URL "original"
#define REDIRECT_URL "redirect"
#define REDIRECT_TIMESTAMP "timestamp"
#define REDIRECT_CACHE_FILENAME "cache.ini"

class PictureLoaderWorkerWork;
inline Q_LOGGING_CATEGORY(PictureLoaderWorkerLog, "picture_loader.worker");

class PictureLoaderWorker : public QObject
{
    Q_OBJECT
public:
    explicit PictureLoaderWorker();
    ~PictureLoaderWorker() override;

    void enqueueImageLoad(const CardInfoPtr &card);
    void clearNetworkCache();

public slots:
    QNetworkReply *makeRequest(const QUrl &url, PictureLoaderWorkerWork *workThread);
    void imageLoadedSuccessfully(CardInfoPtr card, const QImage &image);

private:
    static QStringList md5Blacklist;

    QThread *pictureLoaderThread;
    QString picsPath, customPicsPath;
    QList<PictureToLoad> loadQueue;
    QMutex mutex;
    QNetworkAccessManager *networkManager;
    QHash<QUrl, QPair<QUrl, QDateTime>> redirectCache; // Stores redirect and timestamp
    QString cacheFilePath;                             // Path to persistent storage
    static constexpr int CacheTTLInDays = 30;          // TODO: Make user configurable
    QList<PictureToLoad> cardsToDownload;
    PictureToLoad cardBeingLoaded;
    PictureToLoad cardBeingDownloaded;
    bool picDownload, downloadRunning, loadQueueRunning;

    void cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl);
    QUrl getCachedRedirect(const QUrl &originalUrl) const;
    void loadRedirectCache();
    void saveRedirectCache() const;
    void cleanStaleEntries();

private slots:
    void picDownloadChanged();
    void picsPathChanged();

signals:
    void startLoadQueue();
    void imageLoaded(CardInfoPtr card, const QImage &image);
};

#endif // PICTURE_LOADER_WORKER_H
