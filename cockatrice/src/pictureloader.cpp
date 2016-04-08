#include "pictureloader.h"
#include "carddatabase.h"
#include "main.h"
#include "settingscache.h"
#include "thememanager.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmapCache>
#include <QSet>
#include <QSvgRenderer>
#include <QThread>
#include <QUrl>

// never cache more than 300 cards at once for a single deck
#define CACHED_CARD_PER_DECK_MAX 300

class PictureToLoad::SetDownloadPriorityComparator {
public:
    /*
     * Returns true if a has higher download priority than b
     * Enabled sets have priority over disabled sets
     * Both groups follows the user-defined order
     */
    inline bool operator()(CardSet *a, CardSet *b) const
    {
        if(a->getEnabled())
        {
            if(b->getEnabled())
            {
                // both enabled: sort by key
                return a->getSortKey() < b->getSortKey();
            } else {
                // only a enabled
                return true;
            }
        } else {
            if(b->getEnabled())
            {
                // only b enabled
                return false;
            } else {
                // both disabled: sort by key
                return a->getSortKey() < b->getSortKey();
            }
        }
    }
};

PictureToLoad::PictureToLoad(CardInfo *_card)
    : card(_card), setIndex(0)
{
    if (card) {
        sortedSets = card->getSets();
        qSort(sortedSets.begin(), sortedSets.end(), SetDownloadPriorityComparator());
    }
}

bool PictureToLoad::nextSet()
{
    if (setIndex == sortedSets.size() - 1)
        return false;
    ++setIndex;
    return true;
}

QString PictureToLoad::getSetName() const
{
    if (setIndex < sortedSets.size())
        return sortedSets[setIndex]->getCorrectedShortName();
    else
        return QString("");
}

CardSet *PictureToLoad::getCurrentSet() const
{
    if (setIndex < sortedSets.size())
        return sortedSets[setIndex];
    else
        return 0;
}

QStringList PictureLoaderWorker::md5Blacklist = QStringList()
    << "db0c48db407a907c16ade38de048a441"; // card back returned by gatherer when card is not found

PictureLoaderWorker::PictureLoaderWorker()
    : QObject(0),
      downloadRunning(false), loadQueueRunning(false)
{
    picsPath = settingsCache->getPicsPath();
    customPicsPath = settingsCache->getCustomPicsPath();
    picDownload = settingsCache->getPicDownload();

    connect(this, SIGNAL(startLoadQueue()), this, SLOT(processLoadQueue()), Qt::QueuedConnection);
    connect(settingsCache, SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(settingsCache, SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(picDownloadFinished(QNetworkReply *)));

    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);
}

PictureLoaderWorker::~PictureLoaderWorker()
{
    pictureLoaderThread->deleteLater();
}

void PictureLoaderWorker::processLoadQueue()
{
    if (loadQueueRunning)
        return;

    loadQueueRunning = true;
    forever {
        mutex.lock();
        if (loadQueue.isEmpty()) {
            mutex.unlock();
            loadQueueRunning = false;
            return;
        }
        cardBeingLoaded = loadQueue.takeFirst();
        mutex.unlock();

        QString setName = cardBeingLoaded.getSetName();
        QString correctedCardname = cardBeingLoaded.getCard()->getCorrectedName();
        qDebug() << "Trying to load picture (set: " << setName << " card: " << correctedCardname << ")";

        if(cardImageExistsOnDisk(setName, correctedCardname))
            continue;

        if (picDownload) {
            qDebug() << "Picture NOT found, trying to download (set: " << setName << " card: " << correctedCardname << ")";
            cardsToDownload.append(cardBeingLoaded);
            cardBeingLoaded=0;
            if (!downloadRunning)
                startNextPicDownload();
        } else {
            if (cardBeingLoaded.nextSet())
            {
                qDebug() << "Picture NOT found and download disabled, moving to next set (newset: " << setName << " card: " << correctedCardname << ")";
                mutex.lock();
                loadQueue.prepend(cardBeingLoaded);
                cardBeingLoaded=0;
                mutex.unlock();
            } else {
                qDebug() << "Picture NOT found, download disabled, no more sets to try: BAILING OUT (oldset: " << setName << " card: " << correctedCardname << ")";
                imageLoaded(cardBeingLoaded.getCard(), QImage());
            }
        }
    }
}

bool PictureLoaderWorker::cardImageExistsOnDisk(QString & setName, QString & correctedCardname)
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);

    //The list of paths to the folders in which to search for images
    QList<QString> picsPaths = QList<QString>() << customPicsPath + correctedCardname;

    if(!setName.isEmpty())
    {
        picsPaths   << picsPath + "/" + setName + "/" + correctedCardname
                    << picsPath + "/downloadedPics/" + setName + "/" + correctedCardname;
    }

    //Iterates through the list of paths, searching for images with the desired name with any QImageReader-supported extension
    for (int i = 0; i < picsPaths.length(); i ++) {
        imgReader.setFileName(picsPaths.at(i));
        if (imgReader.read(&image)) {
            qDebug() << "Picture found on disk (set: " << setName << " card: " << correctedCardname << ")";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(picsPaths.at(i) + ".full");
        if (imgReader.read(&image)) {
            qDebug() << "Picture.full found on disk (set: " << setName << " card: " << correctedCardname << ")";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
    }

    return false;
}

QString PictureLoaderWorker::getPicUrl()
{
    if (!picDownload) return QString();

    CardInfo *card = cardBeingDownloaded.getCard();
    CardSet *set=cardBeingDownloaded.getCurrentSet();
    QString picUrl = QString("");

    // if sets have been defined for the card, they can contain custom picUrls
    if(set)
    {
        picUrl = card->getCustomPicURL(set->getShortName());
        if (!picUrl.isEmpty())
            return picUrl;
    }

    // if a card has a muid, use the default url; if not, use the fallback
    int muid = set ? card->getMuId(set->getShortName()) : 0;
    picUrl = muid ? settingsCache->getPicUrl() : settingsCache->getPicUrlFallback();

    picUrl.replace("!name!", QUrl::toPercentEncoding(card->getCorrectedName()));
    picUrl.replace("!name_lower!", QUrl::toPercentEncoding(card->getCorrectedName().toLower()));
    picUrl.replace("!cardid!", QUrl::toPercentEncoding(QString::number(muid)));
    if (set)
    {
        picUrl.replace("!setnumber!", QUrl::toPercentEncoding(card->getSetNumber(set->getShortName())));
        picUrl.replace("!setcode!", QUrl::toPercentEncoding(set->getShortName()));
        picUrl.replace("!setcode_lower!", QUrl::toPercentEncoding(set->getShortName().toLower()));
        picUrl.replace("!setname!", QUrl::toPercentEncoding(set->getLongName()));
        picUrl.replace("!setname_lower!", QUrl::toPercentEncoding(set->getLongName().toLower()));
    }

    if (
        picUrl.contains("!name!") ||
        picUrl.contains("!name_lower!") ||
        picUrl.contains("!setnumber!") ||
        picUrl.contains("!setcode!") ||
        picUrl.contains("!setcode_lower!") ||
        picUrl.contains("!setname!") ||
        picUrl.contains("!setname_lower!") ||
        picUrl.contains("!cardid!")
        )
    {
        qDebug() << "Insufficient card data to download" << card->getName() << "Url:" << picUrl;
        return QString();
    }

    return picUrl;
}

void PictureLoaderWorker::startNextPicDownload()
{
    if (cardsToDownload.isEmpty()) {
        cardBeingDownloaded = 0;
        downloadRunning = false;
        return;
    }

    downloadRunning = true;

    cardBeingDownloaded = cardsToDownload.takeFirst();

    QString picUrl = getPicUrl();
    if (picUrl.isEmpty()) {
        downloadRunning = false;
        picDownloadFailed();
    } else {
        QUrl url(picUrl);

        QNetworkRequest req(url);
        qDebug() << "starting picture download:" << cardBeingDownloaded.getCard()->getName() << "Url:" << req.url();
        networkManager->get(req);
    }
}

void PictureLoaderWorker::picDownloadFailed()
{
    if (cardBeingDownloaded.nextSet())
    {
        qDebug() << "Picture NOT found, download failed, moving to next set (newset: " << cardBeingDownloaded.getSetName() << " card: " << cardBeingDownloaded.getCard()->getCorrectedName() << ")";
        mutex.lock();
        loadQueue.prepend(cardBeingDownloaded);
        mutex.unlock();
    } else {
        qDebug() << "Picture NOT found, download failed, no more sets to try: BAILING OUT (oldset: " << cardBeingDownloaded.getSetName() << " card: " << cardBeingDownloaded.getCard()->getCorrectedName() << ")";
        imageLoaded(cardBeingDownloaded.getCard(), QImage());
        cardBeingDownloaded = 0;
    }
    emit startLoadQueue();
}

bool PictureLoaderWorker::imageIsBlackListed(const QByteArray &picData)
{
    QString md5sum = QCryptographicHash::hash(picData, QCryptographicHash::Md5).toHex();
    return md5Blacklist.contains(md5sum);
}

void PictureLoaderWorker::picDownloadFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << "Download failed:" << reply->errorString();
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QNetworkRequest req(redirectUrl);
        qDebug() << "following redirect:" << cardBeingDownloaded.getCard()->getName() << "Url:" << req.url();
        networkManager->get(req);
        return;
    }

    const QByteArray &picData = reply->peek(reply->size()); //peek is used to keep the data in the buffer for use by QImageReader

    if(imageIsBlackListed(picData))
    {
        qDebug() << "Picture downloaded, but blacklisted, will consider it as not found";
        picDownloadFailed();
        reply->deleteLater();
        startNextPicDownload();
        return;
    }

    QImage testImage;
    
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(reply);
    QString extension = "." + imgReader.format(); //the format is determined prior to reading the QImageReader data into a QImage object, as that wipes the QImageReader buffer
    if (extension == ".jpeg")
        extension = ".jpg";
    
    if (imgReader.read(&testImage)) {
        QString setName = cardBeingDownloaded.getSetName();
        if(!setName.isEmpty())
        {
            if (!QDir().mkpath(picsPath + "/downloadedPics/" + setName)) {
                qDebug() << picsPath + "/downloadedPics/" + setName + " could not be created.";
                return;
            }

            QFile newPic(picsPath + "/downloadedPics/" + setName + "/" + cardBeingDownloaded.getCard()->getCorrectedName() + extension);
            if (!newPic.open(QIODevice::WriteOnly))
                return;
            newPic.write(picData);
            newPic.close();
        }

        imageLoaded(cardBeingDownloaded.getCard(), testImage);
    } else {
        picDownloadFailed();
    } 

    reply->deleteLater();
    startNextPicDownload();
}

void PictureLoaderWorker::enqueueImageLoad(CardInfo *card)
{
    QMutexLocker locker(&mutex);

    // avoid queueing the same card more than once
    if(!card || card == cardBeingLoaded.getCard() || card == cardBeingDownloaded.getCard())
        return;

    foreach(PictureToLoad pic, loadQueue)
    {
        if(pic.getCard() == card)
            return;
    }

    foreach(PictureToLoad pic, cardsToDownload)
    {
        if(pic.getCard() == card)
            return;
    }

    loadQueue.append(PictureToLoad(card));
    emit startLoadQueue();
}

void PictureLoaderWorker::picDownloadChanged()
{
    QMutexLocker locker(&mutex);
    picDownload = settingsCache->getPicDownload();
}

void PictureLoaderWorker::picsPathChanged()
{
    QMutexLocker locker(&mutex);
    picsPath = settingsCache->getPicsPath();
    customPicsPath = settingsCache->getCustomPicsPath();
}

PictureLoader::PictureLoader()
    : QObject(0)
{
    worker = new PictureLoaderWorker;
    connect(settingsCache, SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(settingsCache, SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    connect(worker, SIGNAL(imageLoaded(CardInfo *, const QImage &)), this, SLOT(imageLoaded(CardInfo *, const QImage &)));
}

PictureLoader::~PictureLoader()
{
    worker->deleteLater();
}

void PictureLoader::internalGetCardBackPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + QString::number(size.height());
    if(!QPixmapCache::find(backCacheKey, &pixmap))
    {
        qDebug() << "cache fail for" << backCacheKey;
        pixmap = QPixmap("theme:cardback").scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }    
}

void PictureLoader::getPixmap(QPixmap &pixmap, CardInfo *card, QSize size)
{
    if(card)
    {    
        // search for an exact size copy of the picure in cache
        QString key = card->getPixmapCacheKey();
        QString sizekey = key + QLatin1Char('_') + QString::number(size.width()) + QString::number(size.height());
        if(QPixmapCache::find(sizekey, &pixmap))
            return;

        // load the image and create a copy of the correct size
        QPixmap bigPixmap;
        if(QPixmapCache::find(key, &bigPixmap))
        {
            pixmap = bigPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmapCache::insert(sizekey, pixmap);
            return;
        }

        // add the card to the load queue
        getInstance().worker->enqueueImageLoad(card);
    } else {
        // requesting the image for a null card is a shortcut to get the card background image
        internalGetCardBackPixmap(pixmap, size);
    }
}

void PictureLoader::imageLoaded(CardInfo *card, const QImage &image)
{
    if(image.isNull())
    {
        QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap());
    } else {
        if(card->getUpsideDownArt())
        {
            QImage mirrorImage = image.mirrored(true, true);
            QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap::fromImage(mirrorImage));
        } else {
            QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap::fromImage(image));
        }
    }

    card->emitPixmapUpdated();
}

void PictureLoader::clearPixmapCache(CardInfo *card)
{
    if(card)
        QPixmapCache::remove(card->getPixmapCacheKey());
}

void PictureLoader::clearPixmapCache()
{
    QPixmapCache::clear();
}

void PictureLoader::cacheCardPixmaps(QList<CardInfo *> cards)
{
    QPixmap tmp;
    int max = qMin(cards.size(), CACHED_CARD_PER_DECK_MAX);
    for (int i = 0; i < max; ++i)
    {
        CardInfo * card = cards.at(i);
        if(!card)
            continue;

        QString key = card->getPixmapCacheKey();
        if(QPixmapCache::find(key, &tmp))
            continue;

        getInstance().worker->enqueueImageLoad(card);
    }
}

void PictureLoader::picDownloadChanged()
{
    QPixmapCache::clear();
}

void PictureLoader::picsPathChanged()
{
    QPixmapCache::clear();
}