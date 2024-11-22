#include "picture_loader.h"

#include "../../game/cards/card_database.h"
#include "../../game/cards/card_database_manager.h"
#include "../../settings/cache_settings.h"
#include "theme_manager.h"

#include <QApplication>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QImageReader>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmapCache>
#include <QRegularExpression>
#include <QScreen>
#include <QSet>
#include <QSvgRenderer>
#include <QThread>
#include <QUrl>
#include <algorithm>
#include <utility>

// never cache more than 300 cards at once for a single deck
#define CACHED_CARD_PER_DECK_MAX 300

PictureToLoad::PictureToLoad(CardInfoPtr _card)
    : card(std::move(_card)), urlTemplates(SettingsCache::instance().downloads().getAllURLs())
{
    if (card) {
        for (const auto &set : card->getSets()) {
            sortedSets << set.getPtr();
        }
        if (sortedSets.empty()) {
            sortedSets << CardSet::newInstance("", "", "", QDate());
        }
        std::sort(sortedSets.begin(), sortedSets.end(), SetDownloadPriorityComparator());
        // If the pixmapCacheKey corresponds to a specific set, we have to try to load it first.
        for (const auto &set : card->getSets()) {
            if (QLatin1String("card_") + card->getName() + QString("_") + QString(set.getProperty("uuid")) ==
                card->getPixmapCacheKey()) {
                long long setIndex = sortedSets.indexOf(set.getPtr());
                CardSetPtr setForCardProviderID = sortedSets.takeAt(setIndex);
                sortedSets.prepend(setForCardProviderID);
            }
        }
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

PictureLoaderWorker::PictureLoaderWorker()
    : QObject(nullptr), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath()),
      picDownload(SettingsCache::instance().getPicDownload()), downloadRunning(false), loadQueueRunning(false)
{
    connect(this, SIGNAL(startLoadQueue()), this, SLOT(processLoadQueue()), Qt::QueuedConnection);
    connect(&SettingsCache::instance(), SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    networkManager = new QNetworkAccessManager(this);
    // We need a timeout to ensure requests don't hang indefinitely in case of
    // cache corruption, see related Qt bug: https://bugreports.qt.io/browse/QTBUG-111397
    // Use Qt's default timeout (30s, as of 2023-02-22)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    networkManager->setTransferTimeout();
#endif
    auto cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(SettingsCache::instance().getNetworkCachePath());
    // Note: the settings is in MB, but QNetworkDiskCache uses bytes
    connect(&SettingsCache::instance(), &SettingsCache::networkCacheSizeChanged, cache,
            [cache](int newSizeInMB) { cache->setMaximumCacheSize(1024L * 1024L * static_cast<qint64>(newSizeInMB)); });
    networkManager->setCache(cache);
    // Use a ManualRedirectPolicy since we keep track of redirects in picDownloadFinished
    // We can't use NoLessSafeRedirectPolicy because it is not applied with AlwaysCache
    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(picDownloadFinished(QNetworkReply *)));

    cacheFilePath = SettingsCache::instance().getRedirectCachePath() + "cacheSettings.ini";
    loadRedirectCache();
    cleanStaleEntries();

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this,
            &PictureLoaderWorker::saveRedirectCache);

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

        qDebug().nospace() << "PictureLoader: [card: " << cardName << " set: " << setName
                           << "]: Trying to load picture";

        if (CardDatabaseManager::getInstance()->isProviderIdForPreferredPrinting(
                cardName, cardBeingLoaded.getCard()->getPixmapCacheKey())) {
            if (cardImageExistsOnDisk(setName, correctedCardName)) {
                continue;
            }
        }

        qDebug().nospace() << "PictureLoader: [card: " << cardName << " set: " << setName
                           << "]: No custom picture, trying to download";
        cardsToDownload.append(cardBeingLoaded);
        cardBeingLoaded.clear();
        if (!downloadRunning) {
            startNextPicDownload();
        }
    }
}

bool PictureLoaderWorker::cardImageExistsOnDisk(QString &setName, QString &correctedCardname)
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    QList<QString> picsPaths = QList<QString>();
    QDirIterator it(customPicsPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    // Recursively check all subdirectories of the CUSTOM folder
    while (it.hasNext()) {
        QString thisPath(it.next());
        QFileInfo thisFileInfo(thisPath);

        if (thisFileInfo.isFile() &&
            (thisFileInfo.fileName() == correctedCardname || thisFileInfo.completeBaseName() == correctedCardname ||
             thisFileInfo.baseName() == correctedCardname)) {
            picsPaths << thisPath; // Card found in the CUSTOM directory, somewhere
        }
    }

    if (!setName.isEmpty()) {
        picsPaths << picsPath + "/" + setName + "/" + correctedCardname
                  // We no longer store downloaded images there, but don't just ignore
                  // stuff that old versions have put there.
                  << picsPath + "/downloadedPics/" + setName + "/" + correctedCardname;
    }

    // Iterates through the list of paths, searching for images with the desired
    // name with any QImageReader-supported
    // extension
    for (const auto &_picsPath : picsPaths) {
        imgReader.setFileName(_picsPath);
        if (imgReader.read(&image)) {
            qDebug().nospace() << "PictureLoader: [card: " << correctedCardname << " set: " << setName
                               << "]: Picture found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(_picsPath + ".full");
        if (imgReader.read(&image)) {
            qDebug().nospace() << "PictureLoader: [card: " << correctedCardname << " set: " << setName
                               << "]: Picture.full found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(_picsPath + ".xlhq");
        if (imgReader.read(&image)) {
            qDebug().nospace() << "PictureLoader: [card: " << correctedCardname << " set: " << setName
                               << "]: Picture.xlhq found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
    }

    return false;
}

static int parse(const QString &urlTemplate,
                 const QString &propType,
                 const QString &cardName,
                 const QString &setName,
                 std::function<QString(const QString &)> getProperty,
                 QMap<QString, QString> &transformMap)
{
    static const QRegularExpression rxFillWith("^(.+)_fill_with_(.+)$");
    static const QRegularExpression rxSubStr("^(.+)_substr_(\\d+)_(\\d+)$");

    const QRegularExpression rxCardProp("!" + propType + ":([^!]+)!");

    auto matches = rxCardProp.globalMatch(urlTemplate);
    while (matches.hasNext()) {
        auto match = matches.next();
        QString templatePropertyName = match.captured(1);
        auto fillMatch = rxFillWith.match(templatePropertyName);
        QString cardPropertyName;
        QString fillWith;
        int subStrPos = 0;
        int subStrLen = -1;
        if (fillMatch.hasMatch()) {
            cardPropertyName = fillMatch.captured(1);
            fillWith = fillMatch.captured(2);
        } else {
            fillWith = QString();
            auto subStrMatch = rxSubStr.match(templatePropertyName);
            if (subStrMatch.hasMatch()) {
                cardPropertyName = subStrMatch.captured(1);
                subStrPos = subStrMatch.captured(2).toInt();
                subStrLen = subStrMatch.captured(3).toInt();
            } else {
                cardPropertyName = templatePropertyName;
            }
        }
        QString propertyValue = getProperty(cardPropertyName);
        if (propertyValue.isEmpty()) {
            qDebug().nospace() << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested "
                               << propType << "property (" << cardPropertyName << ") for Url template (" << urlTemplate
                               << ") is not available";
            return 1;
        } else {
            int propLength = propertyValue.length();
            if (subStrLen > 0) {
                if (subStrPos + subStrLen > propLength) {
                    qDebug().nospace() << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested "
                                       << propType << " property (" << cardPropertyName << ") for Url template ("
                                       << urlTemplate << ") is smaller than substr specification (" << subStrPos
                                       << " + " << subStrLen << " > " << propLength << ")";
                    return 1;
                } else {
                    propertyValue = propertyValue.mid(subStrPos, subStrLen);
                    propLength = subStrLen;
                }
            }

            if (!fillWith.isEmpty()) {
                int fillLength = fillWith.length();
                if (fillLength < propLength) {
                    qDebug().nospace() << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested "
                                       << propType << " property (" << cardPropertyName << ") for Url template ("
                                       << urlTemplate << ") is longer than fill specification (" << fillWith << ")";
                    return 1;
                } else {

                    propertyValue = fillWith.left(fillLength - propLength) + propertyValue;
                }
            }

            transformMap["!" + propType + ":" + templatePropertyName + "!"] = propertyValue;
        }
    }
    return 0;
}

QString PictureToLoad::transformUrl(const QString &urlTemplate) const
{
    /* This function takes Url templates and substitutes actual card details
       into the url.  This is used for making Urls with follow a predictable format
       for downloading images.  If information is requested by the template that is
       not populated for this specific card/set combination, an empty string is returned.*/

    CardSetPtr set = getCurrentSet();

    QMap<QString, QString> transformMap = QMap<QString, QString>();
    QString setName = getSetName();

    // name
    QString cardName = card->getName();
    transformMap["!name!"] = cardName;
    transformMap["!name_lower!"] = card->getName().toLower();
    transformMap["!corrected_name!"] = card->getCorrectedName();
    transformMap["!corrected_name_lower!"] = card->getCorrectedName().toLower();

    // card properties
    if (parse(
            urlTemplate, "prop", cardName, setName, [&](const QString &name) { return card->getProperty(name); },
            transformMap)) {
        return QString();
    }

    if (set) {
        transformMap["!setcode!"] = set->getShortName();
        transformMap["!setcode_lower!"] = set->getShortName().toLower();
        transformMap["!setname!"] = set->getLongName();
        transformMap["!setname_lower!"] = set->getLongName().toLower();

        if (parse(
                urlTemplate, "set", cardName, setName,
                [&](const QString &name) { return card->getSetProperty(set->getShortName(), name); }, transformMap)) {
            return QString();
        }
    }

    // language setting
    transformMap["!sflang!"] = QString(QCoreApplication::translate(
        "PictureLoader", "en", "code for scryfall's language property, not available for all languages"));

    QString transformedUrl = urlTemplate;
    for (const QString &prop : transformMap.keys()) {
        if (transformedUrl.contains(prop)) {
            if (!transformMap[prop].isEmpty()) {
                transformedUrl.replace(prop, QUrl::toPercentEncoding(transformMap[prop]));
            } else {
                /* This means the template is requesting information that is not
                 * populated in this card, so it should return an empty string,
                 * indicating an invalid Url.
                 */
                qDebug().nospace() << "PictureLoader: [card: " << cardName << " set: " << setName
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
        qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getCorrectedName()
                           << " set: " << cardBeingDownloaded.getSetName() << "]: Trying to fetch picture from url "
                           << url.toDisplayString();
        makeRequest(url);
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
        qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getCorrectedName()
                           << " set: " << cardBeingDownloaded.getSetName() << "]: Picture NOT found, "
                           << (picDownload ? "download failed" : "downloads disabled")
                           << ", no more url combinations to try: BAILING OUT";
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

QNetworkReply *PictureLoaderWorker::makeRequest(const QUrl &url)
{
    // Check if the redirect is cached
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        qDebug() << "Using cached redirect for" << url << "to" << cachedRedirect;
        return makeRequest(cachedRedirect); // Use the cached redirect
    }

    QNetworkRequest req(url);

    if (!picDownload) {
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }

    QNetworkReply *reply = networkManager->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if (redirectTarget.isValid()) {
            QUrl redirectUrl = redirectTarget.toUrl();
            if (redirectUrl.isRelative()) {
                redirectUrl = url.resolved(redirectUrl);
            }

            cacheRedirect(url, redirectUrl);
            qDebug() << "Caching redirect from" << url << "to" << redirectUrl;
        }

        reply->deleteLater();
    });

    return reply;
}

void PictureLoaderWorker::cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl)
{
    redirectCache[originalUrl] = qMakePair(redirectUrl, QDateTime::currentDateTimeUtc());
    saveRedirectCache();
}

QUrl PictureLoaderWorker::getCachedRedirect(const QUrl &originalUrl) const
{
    if (redirectCache.contains(originalUrl)) {
        return redirectCache[originalUrl].first;
    }
    return QUrl();
}

void PictureLoaderWorker::loadRedirectCache()
{
    QSettings settings(cacheFilePath, QSettings::IniFormat);

    redirectCache.clear();
    int size = settings.beginReadArray("redirects");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QUrl originalUrl = settings.value("original").toUrl();
        QUrl redirectUrl = settings.value("redirect").toUrl();
        QDateTime timestamp = settings.value("timestamp").toDateTime();

        if (originalUrl.isValid() && redirectUrl.isValid()) {
            redirectCache[originalUrl] = qMakePair(redirectUrl, timestamp);
        }
    }
    settings.endArray();
}

void PictureLoaderWorker::saveRedirectCache() const
{
    qDebug() << "Saving redirect cache";
    QSettings settings(cacheFilePath, QSettings::IniFormat);

    settings.beginWriteArray("redirects", redirectCache.size());
    int index = 0;
    for (auto it = redirectCache.cbegin(); it != redirectCache.cend(); ++it) {
        settings.setArrayIndex(index++);
        settings.setValue("original", it.key());
        settings.setValue("redirect", it.value().first);
        settings.setValue("timestamp", it.value().second);
    }
    settings.endArray();
}

void PictureLoaderWorker::cleanStaleEntries()
{
    QDateTime now = QDateTime::currentDateTimeUtc();

    auto it = redirectCache.begin();
    while (it != redirectCache.end()) {
        if (it.value().second.addDays(CacheTTLInDays) < now) {
            it = redirectCache.erase(it); // Remove stale entry
        } else {
            ++it;
        }
    }
}

void PictureLoaderWorker::picDownloadFinished(QNetworkReply *reply)
{
    bool isFromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

    if (reply->error()) {
        if (isFromCache) {
            qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                               << " set: " << cardBeingDownloaded.getSetName()
                               << "]: Removing corrupted cache file for url " << reply->url().toDisplayString()
                               << " and retrying (" << reply->errorString() << ")";

            networkManager->cache()->remove(reply->url());

            makeRequest(reply->url());
        } else {
            qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                               << " set: " << cardBeingDownloaded.getSetName()
                               << "]: " << (picDownload ? "Download" : "Cache search") << " failed for url "
                               << reply->url().toDisplayString() << " (" << reply->errorString() << ")";

            picDownloadFailed();
            startNextPicDownload();
        }

        reply->deleteLater();
        return;
    }

    // List of status codes from https://doc.qt.io/qt-6/qnetworkreply.html#redirected
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 ||
        statusCode == 308) {
        QUrl redirectUrl = reply->header(QNetworkRequest::LocationHeader).toUrl();
        qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                           << " set: " << cardBeingDownloaded.getSetName() << "]: following "
                           << (isFromCache ? "cached redirect" : "redirect") << " to " << redirectUrl.toDisplayString();
        makeRequest(redirectUrl);
        reply->deleteLater();
        return;
    }

    // peek is used to keep the data in the buffer for use by QImageReader
    const QByteArray &picData = reply->peek(reply->size());

    if (imageIsBlackListed(picData)) {
        qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                           << " set: " << cardBeingDownloaded.getSetName()
                           << "]: Picture found, but blacklisted, will consider it as not found";

        picDownloadFailed();
        reply->deleteLater();
        startNextPicDownload();
        return;
    }

    QImage testImage;

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(reply);

    bool logSuccessMessage = false;

    static const int riffHeaderSize = 12; // RIFF_HEADER_SIZE from webp/format_constants.h
    auto replyHeader = reply->peek(riffHeaderSize);

    if (replyHeader.startsWith("RIFF") && replyHeader.endsWith("WEBP")) {
        auto imgBuf = QBuffer(this);
        imgBuf.setData(reply->readAll());

        auto movie = QMovie(&imgBuf);
        movie.start();
        movie.stop();

        imageLoaded(cardBeingDownloaded.getCard(), movie.currentImage());
        logSuccessMessage = true;
    } else if (imgReader.read(&testImage)) {
        imageLoaded(cardBeingDownloaded.getCard(), testImage);
        logSuccessMessage = true;
    } else {
        qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                           << " set: " << cardBeingDownloaded.getSetName() << "]: Possible "
                           << (isFromCache ? "cached" : "downloaded") << " picture at "
                           << reply->url().toDisplayString() << " could not be loaded: " << reply->errorString();

        picDownloadFailed();
    }

    if (logSuccessMessage) {
        qDebug().nospace() << "PictureLoader: [card: " << cardBeingDownloaded.getCard()->getName()
                           << " set: " << cardBeingDownloaded.getSetName() << "]: Image successfully "
                           << (isFromCache ? "loaded from cached" : "downloaded from") << " url "
                           << reply->url().toDisplayString();
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
    picDownload = SettingsCache::instance().getPicDownload();
}

void PictureLoaderWorker::picsPathChanged()
{
    QMutexLocker locker(&mutex);
    picsPath = SettingsCache::instance().getPicsPath();
    customPicsPath = SettingsCache::instance().getCustomPicsPath();
}

void PictureLoaderWorker::clearNetworkCache()
{
    networkManager->cache()->clear();
}

PictureLoader::PictureLoader() : QObject(nullptr)
{
    worker = new PictureLoaderWorker;
    connect(&SettingsCache::instance(), SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

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
        QScreen *screen = qApp->primaryScreen();
        qreal dpr = screen->devicePixelRatio();
        pixmap = bigPixmap.scaled(size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(dpr);
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

void PictureLoader::clearNetworkCache()
{
    getInstance().worker->clearNetworkCache();
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
