#ifndef PICTURELOADER_H
#define PICTURELOADER_H

#include "../../game/cards/card_database.h"

#include <QList>
#include <QMap>
#include <QMutex>
#include <QNetworkRequest>
class QNetworkAccessManager;
class QNetworkReply;
class QThread;

#define REDIRECT_HEADER_NAME "redirects"
#define REDIRECT_ORIGINAL_URL "original"
#define REDIRECT_URL "redirect"
#define REDIRECT_TIMESTAMP "timestamp"
#define REDIRECT_CACHE_FILENAME "cache.ini"

class PictureToLoad
{
private:
    class SetDownloadPriorityComparator
    {
    public:
        /*
         * Returns true if a has higher download priority than b
         * Enabled sets have priority over disabled sets
         * Both groups follows the user-defined order
         */
        inline bool operator()(const CardSetPtr &a, const CardSetPtr &b) const
        {
            if (a->getEnabled()) {
                return !b->getEnabled() || a->getSortKey() < b->getSortKey();
            } else {
                return !b->getEnabled() && a->getSortKey() < b->getSortKey();
            }
        }
    };

    CardInfoPtr card;
    QList<CardSetPtr> sortedSets;
    QList<QString> urlTemplates;
    QList<QString> currentSetUrls;
    QString currentUrl;
    CardSetPtr currentSet;

public:
    explicit PictureToLoad(CardInfoPtr _card = CardInfoPtr());

    CardInfoPtr getCard() const
    {
        return card;
    }
    void clear()
    {
        card.clear();
    }
    QString getCurrentUrl() const
    {
        return currentUrl;
    }
    CardSetPtr getCurrentSet() const
    {
        return currentSet;
    }
    QString getSetName() const;
    QString transformUrl(const QString &urlTemplate) const;
    bool nextSet();
    bool nextUrl();
    void populateSetUrls();
};

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
    void startNextPicDownload();
    bool cardImageExistsOnDisk(QString &setName, QString &correctedCardName);
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
public slots:
    void processLoadQueue();

signals:
    void startLoadQueue();
    void imageLoaded(CardInfoPtr card, const QImage &image);
};

class PictureLoader : public QObject
{
    Q_OBJECT
public:
    static PictureLoader &getInstance()
    {
        static PictureLoader instance;
        return instance;
    }

private:
    explicit PictureLoader();
    ~PictureLoader() override;
    // Singleton - Don't implement copy constructor and assign operator
    PictureLoader(PictureLoader const &);
    void operator=(PictureLoader const &);

    PictureLoaderWorker *worker;

public:
    static void getPixmap(QPixmap &pixmap, CardInfoPtr card, QSize size);
    static void getCardBackPixmap(QPixmap &pixmap, QSize size);
    static void clearPixmapCache(CardInfoPtr card);
    static void clearPixmapCache();
    static void cacheCardPixmaps(QList<CardInfoPtr> cards);

public slots:
    static void clearNetworkCache();

private slots:
    void picDownloadChanged();
    void picsPathChanged();

public slots:
    void imageLoaded(CardInfoPtr card, const QImage &image);
};
#endif
