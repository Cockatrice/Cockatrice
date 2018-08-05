#include "pictureloader.h"
#include "carddatabase.h"
#include "main.h"
#include "settingscache.h"
#include "thememanager.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
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
#include <utility>

// never cache more than 300 cards at once for a single deck
#define CACHED_CARD_PER_DECK_MAX 300

class PictureToLoad::SetDownloadPriorityComparator
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

PictureToLoad::PictureToLoad(CardInfoPtr _card) : card(std::move(_card)), setIndex(0)
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

CardSetPtr PictureToLoad::getCurrentSet() const
{
    if (setIndex < sortedSets.size())
        return sortedSets[setIndex];
    else
        return {};
}

QStringList PictureLoaderWorker::md5Blacklist =
    QStringList() << "db0c48db407a907c16ade38de048a441"; // card back returned by gatherer when card is not found

PictureLoaderWorker::PictureLoaderWorker() : QObject(nullptr), downloadRunning(false), loadQueueRunning(false)
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
    forever
    {
        mutex.lock();
        if (loadQueue.isEmpty()) {
            mutex.unlock();
            loadQueueRunning = false;
            return;
        }
        cardBeingLoaded = loadQueue.takeFirst();
        mutex.unlock();

        QString setName = cardBeingLoaded.getSetName();
        QString cardName = cardBeingLoaded.getCard()->getName();
        QString correctedCardName = cardBeingLoaded.getCard()->getCorrectedName();
        qDebug() << "Trying to load picture (set: " << setName << " card: " << cardName << ")";

        if (cardImageExistsOnDisk(setName, correctedCardName))
            continue;

        if (picDownload) {
            qDebug() << "Picture NOT found, trying to download (set: " << setName << " card: " << cardName << ")";
            cardsToDownload.append(cardBeingLoaded);
            cardBeingLoaded.clear();
            if (!downloadRunning)
                startNextPicDownload();
        } else {
            if (cardBeingLoaded.nextSet()) {
                qDebug() << "Picture NOT found and download disabled, moving to next set (newset: " << setName
                         << " card: " << cardName << ")";
                mutex.lock();
                loadQueue.prepend(cardBeingLoaded);
                cardBeingLoaded.clear();
                mutex.unlock();
            } else {
                qDebug() << "Picture NOT found, download disabled, no more sets to try: BAILING OUT (oldset: "
                         << setName << " card: " << cardName << ")";
                imageLoaded(cardBeingLoaded.getCard(), QImage());
            }
        }
    }
}

bool PictureLoaderWorker::cardImageExistsOnDisk(QString &setName, QString &correctedCardname)
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    QList<QString> picsPaths = QList<QString>();
    QDirIterator it(customPicsPath, QDirIterator::Subdirectories);

    // Recursively check all subdirectories of the CUSTOM folder
    while (it.hasNext()) {
        QString thisPath(it.next());
        QFileInfo thisFileInfo(thisPath);

        if (thisFileInfo.isFile() && thisFileInfo.baseName() == correctedCardname)
            picsPaths << thisPath; // Card found in the CUSTOM directory, somewhere
    }

    if (!setName.isEmpty()) {
        picsPaths << picsPath + "/" + setName + "/" + correctedCardname
                  << picsPath + "/downloadedPics/" + setName + "/" + correctedCardname;
    }

    // Iterates through the list of paths, searching for images with the desired name with any QImageReader-supported
    // extension
    for (int i = 0; i < picsPaths.length(); i++) {
        imgReader.setFileName(picsPaths.at(i));
        if (imgReader.read(&image)) {
            qDebug() << "Picture found on disk (set: " << setName << " file: " << correctedCardname << ")";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(picsPaths.at(i) + ".full");
        if (imgReader.read(&image)) {
            qDebug() << "Picture.full found on disk (set: " << setName << " file: " << correctedCardname << ")";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(picsPaths.at(i) + ".xlhq");
        if (imgReader.read(&image)) {
            qDebug() << "Picture.xlhq found on disk (set: " << setName << " file: " << correctedCardname << ")";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
    }

    return false;
}

QList<QString> PictureLoaderWorker::getAllPicUrls()
{
    if (!picDownload)
        return QList<QString>();

    CardInfoPtr card = cardBeingDownloaded.getCard();
    CardSetPtr set = cardBeingDownloaded.getCurrentSet();
    QList<QString> urls = QList<QString>();

    // if sets have been defined for the card, they can contain custom picUrls
    if (set) {
        QString setCustomURL = card->getCustomPicURL(set->getShortName());

        if (!setCustomURL.isEmpty()) {
            urls.append(setCustomURL);
            return urls;
        }
    }

    // if a card has a muid, use the default url; if not, use the fallback
    int muid = set ? card->getMuId(set->getShortName()) : 0;

    QList<QString> urlArray = QList<QString>();
    urlArray.append(settingsCache->getPicUrl());
    urlArray.append(settingsCache->getPicUrlFallback());

    for(int i=0; i < 2; i++) {
        QString originalUrl = urlArray[i];
        QString thisUrl = urlArray[i];
        thisUrl.replace("!name!", QUrl::toPercentEncoding(card->getName()));
        thisUrl.replace("!name_lower!", QUrl::toPercentEncoding(card->getName().toLower()));
        thisUrl.replace("!corrected_name!", QUrl::toPercentEncoding(card->getCorrectedName()));
        thisUrl.replace("!corrected_name_lower!", QUrl::toPercentEncoding(card->getCorrectedName().toLower()));
        thisUrl.replace("!cardid!", QUrl::toPercentEncoding(QString::number(muid)));
        if (set) {
            // renamed from !setnumber! to !collectornumber! on 20160819. Remove the old one when convenient.
            thisUrl.replace("!setnumber!", QUrl::toPercentEncoding(card->getCollectorNumber(set->getShortName())));
            thisUrl.replace("!collectornumber!", QUrl::toPercentEncoding(card->getCollectorNumber(set->getShortName())));

            thisUrl.replace("!setcode!", QUrl::toPercentEncoding(set->getShortName()));
            thisUrl.replace("!setcode_lower!", QUrl::toPercentEncoding(set->getShortName().toLower()));
            thisUrl.replace("!setname!", QUrl::toPercentEncoding(set->getLongName()));
            thisUrl.replace("!setname_lower!", QUrl::toPercentEncoding(set->getLongName().toLower()));
        }

        if (thisUrl.contains("!name!") ||
                thisUrl.contains("!name_lower!") ||
                thisUrl.contains("!corrected_name!") ||
                thisUrl.contains("!corrected_name_lower!") ||
                thisUrl.contains("!setnumber!") ||
                thisUrl.contains("!setcode!") ||
                thisUrl.contains("!setcode_lower!") ||
                thisUrl.contains("!setname!") ||
                thisUrl.contains("!setname_lower!") ||
                thisUrl.contains("!cardid!")) {
            qDebug() << "Insufficient card data to download" << card->getName() << "Url:" << originalUrl;
        }
        else {
            urls.append(thisUrl);
        }

    }

    return urls;
}

void PictureLoaderWorker::startNextPicDownload()
{
    if (cardsToDownload.isEmpty()) {
        cardBeingDownloaded.clear();
        downloadRunning = false;
        return;
    }

    downloadRunning = true;

    cardBeingDownloaded = cardsToDownload.takeFirst();

    QList<QString> picUrls = getAllPicUrls();
    if (picUrls.isEmpty()) {
        downloadRunning = false;
        picDownloadFailed();
    } else {
        QUrl url(picUrls[0]); // For now, just use the first one, like always.

        QNetworkRequest req(url);
        qDebug() << "starting picture download:" << cardBeingDownloaded.getCard()->getName() << "Url:" << req.url();
        networkManager->get(req);
    }
}

void PictureLoaderWorker::picDownloadFailed()
{
    if (cardBeingDownloaded.nextSet()) {
        qDebug() << "Picture NOT found, download failed, moving to next set (newset: "
                 << cardBeingDownloaded.getSetName() << " card: " << cardBeingDownloaded.getCard()->getName() << ")";
        mutex.lock();
        loadQueue.prepend(cardBeingDownloaded);
        mutex.unlock();
    } else {
        qDebug() << "Picture NOT found, download failed, no more sets to try: BAILING OUT (oldset: "
                 << cardBeingDownloaded.getSetName() << " card: " << cardBeingDownloaded.getCard()->getName() << ")";
        imageLoaded(cardBeingDownloaded.getCard(), QImage());
        cardBeingDownloaded.clear();
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

    const QByteArray &picData =
        reply->peek(reply->size()); // peek is used to keep the data in the buffer for use by QImageReader

    if (imageIsBlackListed(picData)) {
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
    QString extension = "." + imgReader.format(); // the format is determined prior to reading the QImageReader data
                                                  // into a QImage object, as that wipes the QImageReader buffer
    if (extension == ".jpeg")
        extension = ".jpg";

    if (imgReader.read(&testImage)) {
        QString setName = cardBeingDownloaded.getSetName();
        if (!setName.isEmpty()) {
            if (!QDir().mkpath(picsPath + "/downloadedPics/" + setName)) {
                qDebug() << picsPath + "/downloadedPics/" + setName + " could not be created.";
                return;
            }

            QFile newPic(picsPath + "/downloadedPics/" + setName + "/" +
                         cardBeingDownloaded.getCard()->getCorrectedName() + extension);
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

void PictureLoaderWorker::enqueueImageLoad(CardInfoPtr card)
{
    QMutexLocker locker(&mutex);

    // avoid queueing the same card more than once
    if (!card || card == cardBeingLoaded.getCard() || card == cardBeingDownloaded.getCard())
        return;

    foreach (PictureToLoad pic, loadQueue) {
        if (pic.getCard() == card)
            return;
    }

    foreach (PictureToLoad pic, cardsToDownload) {
        if (pic.getCard() == card)
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

PictureLoader::PictureLoader() : QObject(0)
{
    worker = new PictureLoaderWorker;
    connect(settingsCache, SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(settingsCache, SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    connect(worker, SIGNAL(imageLoaded(CardInfoPtr, const QImage &)), this,
            SLOT(imageLoaded(CardInfoPtr, const QImage &)));
}

PictureLoader::~PictureLoader()
{
    worker->deleteLater();
}

void PictureLoader::getCardBackPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qDebug() << "cache fail for" << backCacheKey;
        pixmap = QPixmap("theme:cardback").scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void PictureLoader::getPixmap(QPixmap &pixmap, CardInfoPtr card, QSize size)
{
    if (card == nullptr)
        return;

    // search for an exact size copy of the picure in cache
    QString key = card->getPixmapCacheKey();
    QString sizekey = key + QLatin1Char('_') + QString::number(size.width()) + QString::number(size.height());
    if (QPixmapCache::find(sizekey, &pixmap))
        return;

    // load the image and create a copy of the correct size
    QPixmap bigPixmap;
    if (QPixmapCache::find(key, &bigPixmap)) {
        pixmap = bigPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(sizekey, pixmap);
        return;
    }

    // add the card to the load queue
    getInstance().worker->enqueueImageLoad(card);
}

void PictureLoader::imageLoaded(CardInfoPtr card, const QImage &image)
{
    if (image.isNull()) {
        QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap());
    } else {
        if (card->getUpsideDownArt()) {
            QImage mirrorImage = image.mirrored(true, true);
            QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap::fromImage(mirrorImage));
        } else {
            QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap::fromImage(image));
        }
    }

    card->emitPixmapUpdated();
}

void PictureLoader::clearPixmapCache(CardInfoPtr card)
{
    if (card)
        QPixmapCache::remove(card->getPixmapCacheKey());
}

void PictureLoader::clearPixmapCache()
{
    QPixmapCache::clear();
}

void PictureLoader::cacheCardPixmaps(QList<CardInfoPtr> cards)
{
    QPixmap tmp;
    int max = qMin(cards.size(), CACHED_CARD_PER_DECK_MAX);
    for (int i = 0; i < max; ++i) {
        CardInfoPtr card = cards.at(i);
        if (!card)
            continue;

        QString key = card->getPixmapCacheKey();
        if (QPixmapCache::find(key, &tmp))
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