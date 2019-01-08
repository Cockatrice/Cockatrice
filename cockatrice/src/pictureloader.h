#ifndef PICTURELOADER_H
#define PICTURELOADER_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QNetworkRequest>

#include "carddatabase.h"
class QNetworkAccessManager;
class QNetworkReply;
class QThread;

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

private:
    static QStringList md5Blacklist;

    QThread *pictureLoaderThread;
    QString picsPath, customPicsPath;
    QList<PictureToLoad> loadQueue;
    QMutex mutex;
    QNetworkAccessManager *networkManager;
    QList<PictureToLoad> cardsToDownload;
    PictureToLoad cardBeingLoaded;
    PictureToLoad cardBeingDownloaded;
    bool picDownload, downloadRunning, loadQueueRunning;
    void startNextPicDownload();
    bool cardImageExistsOnDisk(QString &, QString &);
    bool imageIsBlackListed(const QByteArray &);
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
private slots:
    void picDownloadChanged();
    void picsPathChanged();
public slots:
    void imageLoaded(CardInfoPtr card, const QImage &image);
};
#endif
