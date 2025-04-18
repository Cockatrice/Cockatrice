#include "picture_loader_worker.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"

#include <QBuffer>
#include <QDirIterator>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>

// Card back returned by gatherer when card is not found
QStringList PictureLoaderWorker::md5Blacklist = QStringList() << "db0c48db407a907c16ade38de048a441";

PictureLoaderWorker::PictureLoaderWorker()
    : QObject(nullptr), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath()),
      picDownload(SettingsCache::instance().getPicDownload()), downloadRunning(false), loadQueueRunning(false),
      overrideAllCardArtWithPersonalPreference(SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference())
{
    connect(this, &PictureLoaderWorker::startLoadQueue, this, &PictureLoaderWorker::processLoadQueue,
            Qt::QueuedConnection);
    connect(&SettingsCache::instance(), &SettingsCache::picsPathChanged, this, &PictureLoaderWorker::picsPathChanged);
    connect(&SettingsCache::instance(), &SettingsCache::picDownloadChanged, this,
            &PictureLoaderWorker::picDownloadChanged);
    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            &PictureLoaderWorker::setOverrideAllCardArtWithPersonalPreference);

    networkManager = new QNetworkAccessManager(this);
    // We need a timeout to ensure requests don't hang indefinitely in case of
    // cache corruption, see related Qt bug: https://bugreports.qt.io/browse/QTBUG-111397
    // Use Qt's default timeout (30s, as of 2023-02-22)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    networkManager->setTransferTimeout();
#endif
    auto cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(SettingsCache::instance().getNetworkCachePath());
    cache->setMaximumCacheSize(1024L * 1024L *
                               static_cast<qint64>(SettingsCache::instance().getNetworkCacheSizeInMB()));
    // Note: the settings is in MB, but QNetworkDiskCache uses bytes
    connect(&SettingsCache::instance(), &SettingsCache::networkCacheSizeChanged, cache,
            [cache](int newSizeInMB) { cache->setMaximumCacheSize(1024L * 1024L * static_cast<qint64>(newSizeInMB)); });
    networkManager->setCache(cache);
    // Use a ManualRedirectPolicy since we keep track of redirects in picDownloadFinished
    // We can't use NoLessSafeRedirectPolicy because it is not applied with AlwaysCache
    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    connect(networkManager, &QNetworkAccessManager::finished, this, &PictureLoaderWorker::picDownloadFinished);

    cacheFilePath = SettingsCache::instance().getRedirectCachePath() + REDIRECT_CACHE_FILENAME;
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

        qCDebug(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardName << " set: " << setName << "]: Trying to load picture";

        // FIXME: This is a hack so that to keep old Cockatrice behavior
        // (ignoring provider ID) when the "override all card art with personal
        // preference" is set.
        //
        // Figure out a proper way to integrate the two systems at some point.
        //
        // Note: need to go through a member for
        // overrideAllCardArtWithPersonalPreference as reading from the
        // SettingsCache instance from the PictureLoaderWorker thread could
        // cause race conditions.
        //
        // XXX: Reading from the CardDatabaseManager instance from the
        // PictureLoaderWorker thread might not be safe either
        bool searchCustomPics = overrideAllCardArtWithPersonalPreference ||
                                CardDatabaseManager::getInstance()->isProviderIdForPreferredPrinting(
                                    cardName, cardBeingLoaded.getCard()->getPixmapCacheKey());
        if (searchCustomPics && cardImageExistsOnDisk(setName, correctedCardName, searchCustomPics)) {
            continue;
        }

        qCDebug(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardName << " set: " << setName << "]: No custom picture, trying to download";
        cardsToDownload.append(cardBeingLoaded);
        cardBeingLoaded.clear();
        if (!downloadRunning) {
            startNextPicDownload();
        }
    }
}

bool PictureLoaderWorker::cardImageExistsOnDisk(QString &setName, QString &correctedCardname, bool searchCustomPics)
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    QList<QString> picsPaths = QList<QString>();

    if (searchCustomPics) {
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
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "[card: " << correctedCardname << " set: " << setName << "]: Picture found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(_picsPath + ".full");
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "[card: " << correctedCardname << " set: " << setName << "]: Picture.full found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
        imgReader.setFileName(_picsPath + ".xlhq");
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "[card: " << correctedCardname << " set: " << setName << "]: Picture.xlhq found on disk.";
            imageLoaded(cardBeingLoaded.getCard(), image);
            return true;
        }
    }

    return false;
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
        qCDebug(PictureLoaderWorkerLog).nospace() << "[card: " << cardBeingDownloaded.getCard()->getCorrectedName()
                                                  << " set: " << cardBeingDownloaded.getSetName()
                                                  << "]: Trying to fetch picture from url " << url.toDisplayString();
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
        qCWarning(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardBeingDownloaded.getCard()->getCorrectedName()
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
        qCDebug(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardBeingDownloaded.getCard()->getCorrectedName()
            << " set: " << cardBeingDownloaded.getSetName() << "]: Using cached redirect for " << url.toDisplayString()
            << " to " << cachedRedirect.toDisplayString();
        return makeRequest(cachedRedirect); // Use the cached redirect
    }

    QNetworkRequest req(url);

    // QNetworkDiskCache leaks file descriptors when downloading compressed
    // files, see https://bugreports.qt.io/browse/QTBUG-135641
    //
    // We can set the Accept-Encoding header manually to disable Qt's automatic
    // response decompression, but then we would have to deal with decompression
    // ourselves.
    //
    // Since we are dowloading images that are usually stored in a
    // compressed format (e.g. jpeg or webp), it's not clear compression at the
    // HTTP level helps; in fact, images are typically returned without
    // compression. Redirects, on the other hand, are compressed and cause file
    // descriptor leaks -- but since redirects have no payload, we don't really
    // care either.
    //
    // In the end, just do the simple thing and disable HTTP compression.
    req.setRawHeader("accept-encoding", "identity");

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
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "[card: " << cardBeingDownloaded.getCard()->getCorrectedName()
                << " set: " << cardBeingDownloaded.getSetName() << "]: Caching redirect from " << url.toDisplayString()
                << " to " << redirectUrl.toDisplayString();
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
    return {};
}

void PictureLoaderWorker::loadRedirectCache()
{
    QSettings settings(cacheFilePath, QSettings::IniFormat);

    redirectCache.clear();
    int size = settings.beginReadArray(REDIRECT_HEADER_NAME);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QUrl originalUrl = settings.value(REDIRECT_ORIGINAL_URL).toUrl();
        QUrl redirectUrl = settings.value(REDIRECT_URL).toUrl();
        QDateTime timestamp = settings.value(REDIRECT_TIMESTAMP).toDateTime();

        if (originalUrl.isValid() && redirectUrl.isValid()) {
            redirectCache[originalUrl] = qMakePair(redirectUrl, timestamp);
        }
    }
    settings.endArray();
}

void PictureLoaderWorker::saveRedirectCache() const
{
    QSettings settings(cacheFilePath, QSettings::IniFormat);

    settings.beginWriteArray(REDIRECT_HEADER_NAME, static_cast<int>(redirectCache.size()));
    int index = 0;
    for (auto it = redirectCache.cbegin(); it != redirectCache.cend(); ++it) {
        settings.setArrayIndex(index++);
        settings.setValue(REDIRECT_ORIGINAL_URL, it.key());
        settings.setValue(REDIRECT_URL, it.value().first);
        settings.setValue(REDIRECT_TIMESTAMP, it.value().second);
    }
    settings.endArray();
}

void PictureLoaderWorker::cleanStaleEntries()
{
    QDateTime now = QDateTime::currentDateTimeUtc();

    auto it = redirectCache.begin();
    while (it != redirectCache.end()) {
        if (it.value().second.addDays(SettingsCache::instance().getRedirectCacheTtl()) < now) {
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
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "[card: " << cardBeingDownloaded.getCard()->getName() << " set: " << cardBeingDownloaded.getSetName()
                << "]: Removing corrupted cache file for url " << reply->url().toDisplayString() << " and retrying ("
                << reply->errorString() << ")";

            networkManager->cache()->remove(reply->url());

            makeRequest(reply->url());
        } else {
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "[card: " << cardBeingDownloaded.getCard()->getName() << " set: " << cardBeingDownloaded.getSetName()
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
        qCDebug(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardBeingDownloaded.getCard()->getName() << " set: " << cardBeingDownloaded.getSetName()
            << "]: following " << (isFromCache ? "cached redirect" : "redirect") << " to "
            << redirectUrl.toDisplayString();
        makeRequest(redirectUrl);
        reply->deleteLater();
        return;
    }

    // peek is used to keep the data in the buffer for use by QImageReader
    const QByteArray &picData = reply->peek(reply->size());

    if (imageIsBlackListed(picData)) {
        qCDebug(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardBeingDownloaded.getCard()->getName() << " set: " << cardBeingDownloaded.getSetName()
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
        qCDebug(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardBeingDownloaded.getCard()->getName() << " set: " << cardBeingDownloaded.getSetName()
            << "]: Possible " << (isFromCache ? "cached" : "downloaded") << " picture at "
            << reply->url().toDisplayString() << " could not be loaded: " << reply->errorString();

        picDownloadFailed();
    }

    if (logSuccessMessage) {
        qCInfo(PictureLoaderWorkerLog).nospace()
            << "[card: " << cardBeingDownloaded.getCard()->getName() << " set: " << cardBeingDownloaded.getSetName()
            << "]: Image successfully " << (isFromCache ? "loaded from cached" : "downloaded from") << " url "
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

void PictureLoaderWorker::setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArtWithPersonalPreference)
{
    overrideAllCardArtWithPersonalPreference = _overrideAllCardArtWithPersonalPreference;
}

void PictureLoaderWorker::clearNetworkCache()
{
    networkManager->cache()->clear();
}
