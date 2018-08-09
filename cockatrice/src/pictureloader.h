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
    class SetDownloadPriorityComparator;

    CardInfoPtr card;
    QList<CardSetPtr> sortedSets;
    QList<QString> urlTemplates;
    QString currentUrl;
    bool customSetPicturesChecked;
    bool urlInitialized;
    int setIndex;
    int urlIndex;

public:
    PictureToLoad(CardInfoPtr _card = CardInfoPtr());
    CardInfoPtr getCard() const
    {
        return card;
    }
    void clear()
    {
        card.clear();
    }
    CardSetPtr getCurrentSet() const;
    QString getCurrentUrl();
    QString getSetName() const;
    QString transformUrl() const;
    bool nextSet();
    bool nextUrl();
};

class PictureLoaderWorker : public QObject
{
    Q_OBJECT
public:
    PictureLoaderWorker();
    ~PictureLoaderWorker();

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
    QList<QString> getAllPicUrls();
    bool cardImageExistsOnDisk(QString &setName, QString &correctedCardname);
    bool imageIsBlackListed(const QByteArray &picData);
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
    PictureLoader();
    ~PictureLoader();
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
