#ifndef PICTURE_LOADER_WORKER_H
#define PICTURE_LOADER_WORKER_H

#include "../../../game/cards/card_database.h"
#include "../../../game/cards/card_info.h"
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
    void queueRequest(const QUrl &url, PictureLoaderWorkerWork *worker);

    void enqueueImageLoad(const CardInfoPtr &card);
    void clearNetworkCache();

public slots:
    QNetworkReply *makeRequest(const QUrl &url, PictureLoaderWorkerWork *workThread);
    void processQueuedRequests();
    void imageLoadedSuccessfully(CardInfoPtr card, const QImage &image);

private:
    static QStringList md5Blacklist;

    QThread *pictureLoaderThread;
    QString picsPath, customPicsPath;
    QList<PictureToLoad> loadQueue;
    QMutex mutex;
    QNetworkAccessManager *networkManager;
    QNetworkDiskCache *cache;
    QHash<QUrl, QPair<QUrl, QDateTime>> redirectCache; // Stores redirect and timestamp
    QString cacheFilePath;                             // Path to persistent storage
    static constexpr int CacheTTLInDays = 30;          // TODO: Make user configurable
    QList<PictureToLoad> cardsToDownload;
    PictureToLoad cardBeingLoaded;
    PictureToLoad cardBeingDownloaded;
    bool picDownload, downloadRunning, loadQueueRunning;
    QQueue<QPair<QUrl, PictureLoaderWorkerWork *>> requestLoadQueue;
    QList<QPair<QUrl, PictureLoaderWorkerWork *>> requestQueue;
    QTimer requestTimer;                    // Timer for processing delayed requests
    bool overrideAllCardArtWithPersonalPreference;

    void cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl);
    QUrl getCachedRedirect(const QUrl &originalUrl) const;
    void loadRedirectCache();
    void saveRedirectCache() const;
    void cleanStaleEntries();

private slots:
    void picDownloadChanged();
    void picsPathChanged();
    void setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArtWithPersonalPreference);

signals:
    void startLoadQueue();
    void imageLoaded(CardInfoPtr card, const QImage &image);
};

#endif // PICTURE_LOADER_WORKER_H
