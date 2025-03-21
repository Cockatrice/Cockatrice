#ifndef PICTURE_LOADER_WORKER_H
#define PICTURE_LOADER_WORKER_H

#include "../../../game/cards/card_database.h"
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

inline Q_LOGGING_CATEGORY(PictureLoaderWorkerLog, "picture_loader.worker");

class PictureLoaderWorker : public QObject
{
    Q_OBJECT
public:
    explicit PictureLoaderWorker();
    ~PictureLoaderWorker() override;

    void enqueueImageLoad(CardInfoPtr card);
    void clearNetworkCache();

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
    bool overrideAllCardArtWithPersonalPreference;
    void startNextPicDownload();

    /** Emit the `imageLoaded` signal and return `true` if a picture is found on
        disk, return `false` otherwise.

        If `searchCustomPics` is `true`, the CUSTOM folder is searched for a
        matching image first; otherwise, only the set-based folders are used. */
    bool cardImageExistsOnDisk(QString &setName, QString &correctedCardName, bool searchCustomPics);

    bool imageIsBlackListed(const QByteArray &);
    QNetworkReply *makeRequest(const QUrl &url);
    void cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl);
    QUrl getCachedRedirect(const QUrl &originalUrl) const;
    void loadRedirectCache();
    void saveRedirectCache() const;
    void cleanStaleEntries();

private slots:
    void picDownloadFinished(QNetworkReply *reply);
    void picDownloadFailed();

    void picDownloadChanged();
    void picsPathChanged();
    void setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArtWithPersonalPreference);
public slots:
    void processLoadQueue();

signals:
    void startLoadQueue();
    void imageLoaded(CardInfoPtr card, const QImage &image);
};

#endif // PICTURE_LOADER_WORKER_H
