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

PictureToLoad::PictureToLoad(CardInfoPtr _card) : card(std::move(_card))
{
    urlTemplates = settingsCache->downloads().getAllURLs();

    if (card) {
        for (const auto &set : card->getSets()) {
            sortedSets << set.getPtr();
        }
        qSort(sortedSets.begin(), sortedSets.end(), SetDownloadPriorityComparator());
        // The first time called, nextSet will also populate the Urls for the first set.
        nextSet();
    }
}

void PictureToLoad::populateSetUrls()
{
    /* currentSetUrls is a list, populated each time a new set is requested for a particular card
       and Urls are removed from it as a download is attempted from each one.  Custom Urls for
       a set are given higher priority, so should be placed first in the list. */
    currentSetUrls.clear();

    if (card && currentSet) {
        QString setCustomURL = card->getCustomPicURL(currentSet->getShortName());

        if (!setCustomURL.isEmpty()) {
            currentSetUrls.append(setCustomURL);
        }
    }

    for (const QString &urlTemplate : urlTemplates) {
        QString transformedUrl = transformUrl(urlTemplate);

        if (!transformedUrl.isEmpty()) {
            currentSetUrls.append(transformedUrl);
        }
    }

    /* Call nextUrl to make sure currentUrl is up-to-date
       but we don't need the result here. */
    (void)nextUrl();
}

bool PictureToLoad::nextSet()
{
    if (!sortedSets.isEmpty()) {
        currentSet = sortedSets.takeFirst();
        populateSetUrls();
        return true;
    }
    currentSet = {};
    return false;
}

bool PictureToLoad::nextUrl()
{
    if (!currentSetUrls.isEmpty()) {
        currentUrl = currentSetUrls.takeFirst();
        return true;
    }
    currentUrl = QString();
    return false;
}

QString PictureToLoad::getSetName() const
{
    if (currentSet) {
        return currentSet->getCorrectedShortName();
    } else {
        return QString();
    }
}

// Card back returned by gatherer when card is not found
QStringList PictureLoaderWorker::md5Blacklist = QStringList() << "db0c48db407a907c16ade38de048a441";

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
    if (loadQueueRunning) {
        return;
    }

    loadQueueRunning = true;
    while (true) {
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

        qDebug() << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Trying to load picture";

        if (cardImageExistsOnDisk(setName, correctedCardName)) {
            continue;
        }

        if (picDownload) {
            qDebug() << "PictureLoader: [card: " << cardName << " set: " << setName
                     << "]: Picture not found on disk, trying to download";
            cardsToDownload.append(cardBeingLoaded);
            cardBeingLoaded.clear();
            if (!downloadRunning) {
                startNextPicDownload();
            }
        } else {
            if (cardBeingLoaded.nextSet()) {
                qDebug() << "PictureLoader: [card: " << cardName << " set: " << setName
                         << "]: Picture NOT found and download disabled, moving to next "
                            "set (new set: "
                         << setName << " card: " << cardName << ")";
                mutex.lock();
                loadQueue.prepend(cardBeingLoaded);
                cardBeingLoaded.clear();
                mutex.unlock();
            } else {
                qDebug() << "PictureLoader: [card: " << cardName << " set: " << setName
                         << "]: Picture NOT found, download disabled, no more sets to "
                            "try: BAILING OUT (old set: "
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

    // Iterates through the list of paths, searching for images with the desired
    // name with any QImageReader-supported
    // extension
    for (const auto &picsPath : picsPaths) {
        imgReader.setFileName(picsPath);
        if (imgReader.read(&image)) {
            qDebug() << "PictureLoader: [card: " << correctedCardname << " set: " << setName
                     << "]: Picture found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(picsPath + ".full");
        if (imgReader.read(&image)) {
            qDebug() << "PictureLoader: [card: " << correctedCardname << " set: " << setName
                     << "]: Picture.full found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(picsPath + ".xlhq");
        if (imgReader.read(&image)) {
            qDebug() << "PictureLoader: [card: " << correctedCardname << " set: " << setName
                     << "]: Picture.xlhq found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
    }

    return false;
}

QString PictureToLoad::transformUrl(const QString &urlTemplate) const
{
    /* This function takes Url templates and substitutes actual card details
       into the url.  This is used for making Urls with follow a predictable format
       for downloading images.  If information is requested by the template that is
       not populated for this specific card/set combination, an empty string is returned.*/

    QString transformedUrl = urlTemplate;
    CardSetPtr set = getCurrentSet();

    QMap<QString, QString> transformMap = QMap<QString, QString>();
    // name
    transformMap["!name!"] = card->getName();
    transformMap["!name_lower!"] = card->getName().toLower();
    transformMap["!corrected_name!"] = card->getCorrectedName();
    transformMap["!corrected_name_lower!"] = card->getCorrectedName().toLower();

    // card properties
    QRegExp rxCardProp("!prop:([^!]+)!");
    int pos = 0;
    while ((pos = rxCardProp.indexIn(transformedUrl, pos)) != -1) {
        QString propertyName = rxCardProp.cap(1);
        pos += rxCardProp.matchedLength();
        QString propertyValue = card->getProperty(propertyName);
        if (propertyValue.isEmpty()) {
            qDebug() << "PictureLoader: [card: " << card->getName() << " set: " << getSetName()
                     << "]: Requested property (" << propertyName << ") for Url template (" << urlTemplate
                     << ") is not available";
            return QString();
        } else {
            transformMap["!prop:" + propertyName + "!"] = propertyValue;
        }
    }

    if (set) {
        transformMap["!setcode!"] = set->getShortName();
        transformMap["!setcode_lower!"] = set->getShortName().toLower();
        transformMap["!setname!"] = set->getLongName();
        transformMap["!setname_lower!"] = set->getLongName().toLower();

        QRegExp rxSetProp("!set:([^!]+)!");
        pos = 0; // Defined above
        while ((pos = rxSetProp.indexIn(transformedUrl, pos)) != -1) {
            QString propertyName = rxSetProp.cap(1);
            pos += rxSetProp.matchedLength();
            QString propertyValue = card->getSetProperty(set->getShortName(), propertyName);
            if (propertyValue.isEmpty()) {
                qDebug() << "PictureLoader: [card: " << card->getName() << " set: " << getSetName()
                         << "]: Requested set property (" << propertyName << ") for Url template (" << urlTemplate
                         << ") is not available";
                return QString();
            } else {
                transformMap["!set:" + propertyName + "!"] = propertyValue;
            }
        }
    }

    for (const QString &prop : transformMap.keys()) {
        if (transformedUrl.contains(prop)) {
            if (!transformMap[prop].isEmpty()) {
                transformedUrl.replace(prop, QUrl::toPercentEncoding(transformMap[prop]));
            } else {
                /* This means the template is requesting information that is not
                 * populated in this card, so it should return an empty string,
                 * indicating an invalid Url.
                 */
                qDebug() << "PictureLoader: [card: " << card->getName() << " set: " << getSetName()
                         << "]: Requested information (" << prop << ") for Url template (" << urlTemplate
                         << ") is not available";
                return QString();
            }
        }
    }

    return transformedUrl;
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

    QString picUrl = cardBeingDownloaded.getCurrentUrl();

    if (picUrl.isEmpty()) {
        downloadRunning = false;
        picDownloadFailed();
    } else {
        QUrl url(picUrl);
        QNetworkRequest req(url);
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getCorrectedName()
                 << " set: " << cardBeingDownloaded.getSetName()
                 << "]: Trying to download picture from url:" << url.toDisplayString();
        networkManager->get(req);
    }
}

void PictureLoaderWorker::picDownloadFailed()
{
    /* Take advantage of short circuiting here to call the nextUrl until one
       is not available.  Only once nextUrl evaluates to false will this move
       on to nextSet.  If the Urls for a particular card are empty, this will
       effectively go through the sets for that card. */
    if (cardBeingDownloaded.nextUrl() || cardBeingDownloaded.nextSet()) {
        mutex.lock();
        loadQueue.prepend(cardBeingDownloaded);
        mutex.unlock();
    } else {
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getCorrectedName()
                 << " set: " << cardBeingDownloaded.getSetName()
                 << "]:  Picture NOT found, download failed, no more url combinations "
                    "to try: BAILING OUT";
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
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                 << " set: " << cardBeingDownloaded.getSetName() << "]:  Download failed:" << reply->errorString();
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QNetworkRequest req(redirectUrl);
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                 << " set: " << cardBeingDownloaded.getSetName() << "]: following redirect:" << req.url().toString();
        networkManager->get(req);
        return;
    }

    // peek is used to keep the data in the buffer for use by QImageReader
    const QByteArray &picData = reply->peek(reply->size());

    if (imageIsBlackListed(picData)) {
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                 << " set: " << cardBeingDownloaded.getSetName()
                 << "]:Picture downloaded, but blacklisted, will consider it as "
                    "not found";
        picDownloadFailed();
        reply->deleteLater();
        startNextPicDownload();
        return;
    }

    QImage testImage;

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(reply);
    QString extension = "." + imgReader.format(); // the format is determined
                                                  // prior to reading the
                                                  // QImageReader data
    // into a QImage object, as that wipes the QImageReader buffer
    if (extension == ".jpeg") {
        extension = ".jpg";
    }

    if (imgReader.read(&testImage)) {
        QString setName = cardBeingDownloaded.getSetName();
        if (!setName.isEmpty()) {
            if (!QDir().mkpath(picsPath + "/downloadedPics/" + setName)) {
                qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                         << " set: " << cardBeingDownloaded.getSetName()
                         << "]: " << picsPath + "/downloadedPics/" + setName + " could not be created.";
                return;
            }

            QFile newPic(picsPath + "/downloadedPics/" + setName + "/" +
                         cardBeingDownloaded.getCard()->getCorrectedName() + extension);
            if (!newPic.open(QIODevice::WriteOnly)) {
                return;
            }
            newPic.write(picData);
            newPic.close();
        }

        imageLoaded(cardBeingDownloaded.getCard(), testImage);
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                 << " set: " << cardBeingDownloaded.getSetName() << "]: Image successfully downloaded from "
                 << reply->request().url().toDisplayString();
    } else {
        qDebug() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                 << " set: " << cardBeingDownloaded.getSetName() << "]: Possible picture at "
                 << reply->request().url().toDisplayString() << " could not be loaded";
        picDownloadFailed();
    }

    reply->deleteLater();
    startNextPicDownload();
}

void PictureLoaderWorker::enqueueImageLoad(CardInfoPtr card)
{
    QMutexLocker locker(&mutex);

    // avoid queueing the same card more than once
    if (!card || card == cardBeingLoaded.getCard() || card == cardBeingDownloaded.getCard()) {
        return;
    }

    for (const PictureToLoad &pic : loadQueue) {
        if (pic.getCard() == card)
            return;
    }

    for (const PictureToLoad &pic : cardsToDownload) {
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

PictureLoader::PictureLoader() : QObject(nullptr)
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
        qDebug() << "PictureLoader: cache fail for" << backCacheKey;
        pixmap = QPixmap("theme:cardback").scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void PictureLoader::getPixmap(QPixmap &pixmap, CardInfoPtr card, QSize size)
{
    if (card == nullptr) {
        return;
    }

    // search for an exact size copy of the picture in cache
    QString key = card->getPixmapCacheKey();
    QString sizeKey = key + QLatin1Char('_') + QString::number(size.width()) + QString::number(size.height());
    if (QPixmapCache::find(sizeKey, &pixmap))
        return;

    // load the image and create a copy of the correct size
    QPixmap bigPixmap;
    if (QPixmapCache::find(key, &bigPixmap)) {
        pixmap = bigPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(sizeKey, pixmap);
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
    if (card) {
        QPixmapCache::remove(card->getPixmapCacheKey());
    }
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
        const CardInfoPtr &card = cards.at(i);
        if (!card) {
            continue;
        }

        QString key = card->getPixmapCacheKey();
        if (QPixmapCache::find(key, &tmp)) {
            continue;
        }

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