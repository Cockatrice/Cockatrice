#include "card_picture_loader_worker.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_loader_local.h"
#include "card_picture_loader_worker_work.h"

#include <QDirIterator>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>
#include <utility>

static constexpr int MAX_REQUESTS_PER_SEC = 10;

CardPictureLoaderWorker::CardPictureLoaderWorker()
    : QObject(nullptr), picDownload(SettingsCache::instance().getPicDownload()), requestQuota(MAX_REQUESTS_PER_SEC)
{
    networkManager = new QNetworkAccessManager(this);
    // We need a timeout to ensure requests don't hang indefinitely in case of
    // cache corruption, see related Qt bug: https://bugreports.qt.io/browse/QTBUG-111397
    // Use Qt's default timeout (30s, as of 2023-02-22)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    networkManager->setTransferTimeout();
#endif
    cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(SettingsCache::instance().getNetworkCachePath());
    cache->setMaximumCacheSize(1024L * 1024L *
                               static_cast<qint64>(SettingsCache::instance().getNetworkCacheSizeInMB()));
    // Note: the settings is in MB, but QNetworkDiskCache uses bytes
    connect(&SettingsCache::instance(), &SettingsCache::networkCacheSizeChanged, this,
            [this](int newSizeInMB) { cache->setMaximumCacheSize(1024L * 1024L * static_cast<qint64>(newSizeInMB)); });
    networkManager->setCache(cache);
    // Use a ManualRedirectPolicy since we keep track of redirects in picDownloadFinished
    // We can't use NoLessSafeRedirectPolicy because it is not applied with AlwaysCache
    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    cacheFilePath = SettingsCache::instance().getRedirectCachePath() + REDIRECT_CACHE_FILENAME;
    loadRedirectCache();
    cleanStaleEntries();

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this,
            &CardPictureLoaderWorker::saveRedirectCache);

    localLoader = new CardPictureLoaderLocal(this);

    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);

    connect(this, &CardPictureLoaderWorker::imageLoadEnqueued, this, &CardPictureLoaderWorker::handleImageLoadEnqueued);

    connect(&requestTimer, &QTimer::timeout, this, &CardPictureLoaderWorker::resetRequestQuota);
    requestTimer.setInterval(1000);
    requestTimer.start();
}

CardPictureLoaderWorker::~CardPictureLoaderWorker()
{
    saveRedirectCache();
    pictureLoaderThread->deleteLater();
}

void CardPictureLoaderWorker::queueRequest(const QUrl &url, CardPictureLoaderWorkerWork *worker)
{
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        queueRequest(cachedRedirect, worker);
    } else if (cache->metaData(url).isValid()) {
        // If we hit a cached url, we get to make the request for free, since it won't contribute towards the rate-limit
        makeRequest(url, worker);
    } else {
        requestLoadQueue.append(qMakePair(url, worker));
        emit imageRequestQueued(url, worker->cardToDownload.getCard(), worker->cardToDownload.getSetName());
        processQueuedRequests();
    }
}

QNetworkReply *CardPictureLoaderWorker::makeRequest(const QUrl &url, CardPictureLoaderWorkerWork *worker)
{
    // Check for cached redirects
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        emit imageRequestSucceeded(url);
        return makeRequest(cachedRedirect, worker);
    }

    QNetworkRequest req(url);
    if (!picDownload) {
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }

    QNetworkReply *reply = networkManager->get(req);

    // Connect reply handling
    connect(reply, &QNetworkReply::finished, worker, [reply, worker] { worker->handleNetworkReply(reply); });

    return reply;
}

void CardPictureLoaderWorker::resetRequestQuota()
{
    requestQuota = MAX_REQUESTS_PER_SEC;
    processQueuedRequests();
}

void CardPictureLoaderWorker::processQueuedRequests()
{
    while (requestQuota > 0 && processSingleRequest()) {
        --requestQuota;
    }
}

bool CardPictureLoaderWorker::processSingleRequest()
{
    if (!requestLoadQueue.isEmpty()) {
        auto request = requestLoadQueue.takeFirst();
        makeRequest(request.first, request.second);
        return true;
    }
    return false;
}

void CardPictureLoaderWorker::enqueueImageLoad(const ExactCard &card)
{
    // Send call through a connection to ensure the handling is run on the pictureLoader thread
    emit imageLoadEnqueued(card);
}

void CardPictureLoaderWorker::handleImageLoadEnqueued(const ExactCard &card)
{
    // deduplicate loads for the same card
    if (currentlyLoading.contains(card.getPixmapCacheKey())) {
        qCDebug(CardPictureLoaderWorkerLog())
            << "Skipping enqueued" << card.getName() << "because it's already being loaded";
        return;
    }
    currentlyLoading.insert(card.getPixmapCacheKey());

    // try to load image from local first
    QImage image = localLoader->tryLoad(card);
    if (!image.isNull()) {
        handleImageLoaded(card, image);
    } else {
        // queue up to load image from remote only after local loading failed
        new CardPictureLoaderWorkerWork(this, card);
    }
}

/**
 * Called when image loading is done. Failures are indicated by an empty QImage.
 */
void CardPictureLoaderWorker::handleImageLoaded(const ExactCard &card, const QImage &image)
{
    currentlyLoading.remove(card.getPixmapCacheKey());
    emit imageLoaded(card, image);
}

void CardPictureLoaderWorker::cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl)
{
    redirectCache[originalUrl] = qMakePair(redirectUrl, QDateTime::currentDateTimeUtc());
    // saveRedirectCache();
}

void CardPictureLoaderWorker::removedCachedUrl(const QUrl &url)
{
    networkManager->cache()->remove(url);
}

QUrl CardPictureLoaderWorker::getCachedRedirect(const QUrl &originalUrl) const
{
    if (redirectCache.contains(originalUrl)) {
        return redirectCache[originalUrl].first;
    }
    return {};
}

void CardPictureLoaderWorker::loadRedirectCache()
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

void CardPictureLoaderWorker::saveRedirectCache() const
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

void CardPictureLoaderWorker::cleanStaleEntries()
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

void CardPictureLoaderWorker::clearNetworkCache()
{
    networkManager->cache()->clear();
    redirectCache.clear();
}
