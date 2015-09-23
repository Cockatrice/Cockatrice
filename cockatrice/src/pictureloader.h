#ifndef PICTURELOADER_H
#define PICTURELOADER_H

#include <QMap>
#include <QList>
#include <QNetworkRequest>
#include <QMutex>

class CardInfo;
class CardSet;
class QNetworkAccessManager;
class QNetworkReply;
class QThread;

class PictureToLoad {
private:
    class EnabledAndKeyCompareFunctor;

    CardInfo *card;
    QList<CardSet *> sortedSets;
    int setIndex;
    bool hq;
public:
    PictureToLoad(CardInfo *_card = 0);
    CardInfo *getCard() const { return card; }
    CardSet *getCurrentSet() const;
    QString getSetName() const;
    bool nextSet();
};

class PictureLoader : public QObject {
Q_OBJECT
public:
    static PictureLoader& getInstance()
    {
        static PictureLoader instance;
        return instance;
    }
private:
    PictureLoader();
    ~PictureLoader();
    // Don't implement
    PictureLoader(PictureLoader const&);
    void operator=(PictureLoader const&); 

    static QStringList md5Blacklist;

    QThread *pictureLoaderThread;
    QString picsPath;
    QList<PictureToLoad> loadQueue;
    QMutex mutex;
    QNetworkAccessManager *networkManager;
    QList<PictureToLoad> cardsToDownload;
    PictureToLoad cardBeingLoaded;
    PictureToLoad cardBeingDownloaded;
    bool picDownload, downloadRunning, loadQueueRunning;
    void startNextPicDownload();
    void imageLoaded(CardInfo *card, const QImage &image);
    QString getPicUrl();
public:
    void enqueueImageLoad(CardInfo *card);
    static void getPixmap(QPixmap &pixmap, CardInfo *card, QSize size);
    static void clearPixmapCache(CardInfo *card);
    static void clearPixmapCache();
    static void cacheCardPixmaps(QList<CardInfo *> cards);
protected:
    static void internalGetCardBackPixmap(QPixmap &pixmap, QSize size);
private slots:
    void picDownloadFinished(QNetworkReply *reply);
    void picDownloadFailed();

    void picDownloadChanged();
    void picsPathChanged();
public slots:
    void processLoadQueue();
signals:
    void startLoadQueue();
};

#endif
