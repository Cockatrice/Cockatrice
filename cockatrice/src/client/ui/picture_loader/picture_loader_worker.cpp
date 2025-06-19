#include "picture_loader_worker.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_loader_local.h"
#include "picture_loader_worker_work.h"

#include <QDirIterator>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>
#include <utility>

// Card back returned by gatherer when card is not found
QStringList PictureLoaderWorker::md5Blacklist = QStringList() << "db0c48db407a907c16ade38de048a441";

PictureLoaderWorker::PictureLoaderWorker() : QObject(nullptr), picDownload(SettingsCache::instance().getPicDownload())
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
            &PictureLoaderWorker::saveRedirectCache);

    localLoader = new PictureLoaderLocal(this);

    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);

    connect(this, &PictureLoaderWorker::imageLoadEnqueued, this, &PictureLoaderWorker::handleImageLoadEnqueued);

    connect(&requestTimer, &QTimer::timeout, this, &PictureLoaderWorker::processQueuedRequests);
    requestTimer.setInterval(1000);
    requestTimer.start();
}

PictureLoaderWorker::~PictureLoaderWorker()
{
    saveRedirectCache();
    pictureLoaderThread->deleteLater();
}

void PictureLoaderWorker::queueRequest(const QUrl &url, PictureLoaderWorkerWork *worker)
{
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        queueRequest(cachedRedirect, worker);
    }
    if (cache->metaData(url).isValid()) {
        makeRequest(url, worker);
    } else {
        requestLoadQueue.append(qMakePair(url, worker));
        emit imageLoadQueued(url, worker);
    }
}

QNetworkReply *PictureLoaderWorker::makeRequest(const QUrl &url, PictureLoaderWorkerWork *worker)
{
    // Check for cached redirects
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        emit imageLoadSuccessful(url, worker);
        return makeRequest(cachedRedirect, worker);
    }

    QNetworkRequest req(url);
    if (!picDownload) {
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }

    QNetworkReply *reply = networkManager->get(req);

    // Connect reply handling
    connect(reply, &QNetworkReply::finished, this, [this, reply, url, worker]() {
        QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (redirectTarget.isValid()) {
            QUrl redirectUrl = redirectTarget.toUrl();
            if (redirectUrl.isRelative()) {
                redirectUrl = url.resolved(redirectUrl);
            }
            cacheRedirect(url, redirectUrl);
        }

        if (reply->error() == QNetworkReply::NoError) {
            worker->picDownloadFinished(reply);
            emit imageLoadSuccessful(url, worker);

            // If we hit a cached image, we get to make another request for free.
            if (reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool()) {
                if (!requestLoadQueue.isEmpty()) {
                    auto request = requestLoadQueue.takeFirst();
                    makeRequest(request.first, request.second);
                }
            }
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 429) {
            qInfo() << "Too many requests.";
        } else {
            worker->picDownloadFinished(reply);
        }
        reply->deleteLater();
    });

    return reply;
}

void PictureLoaderWorker::processQueuedRequests()
{
    for (int i = 0; i < 10; i++) {
        if (!requestLoadQueue.isEmpty()) {
            auto request = requestLoadQueue.takeFirst();
            makeRequest(request.first, request.second);
        }
    }
}

void PictureLoaderWorker::enqueueImageLoad(const CardInfoPtr &card)
{
    // Send call through a connection to ensure the handling is run on the pictureLoader thread
    emit imageLoadEnqueued(card);
}

void PictureLoaderWorker::handleImageLoadEnqueued(const CardInfoPtr &card)
{
    // deduplicate loads for the same card
    if (currentlyLoading.contains(card)) {
        qCDebug(PictureLoaderWorkerLog())
            << "Skipping enqueued" << card->getName() << "because it's already being loaded";
        return;
    }
    currentlyLoading.insert(card);

    // try to load image from local first
    QImage image = localLoader->tryLoad(card);
    if (!image.isNull()) {
        imageLoadedSuccessfully(card, image);
    } else {
        // queue up to load image from remote only after local loading failed
        new PictureLoaderWorkerWork(this, card);
    }
}

/**
 * Called when image loading is done
 * Contrary to the name, this is called on both success and failure. Failures are indicated by an empty QImage.
 */
void PictureLoaderWorker::imageLoadedSuccessfully(const CardInfoPtr &card, const QImage &image)
{
    currentlyLoading.remove(card);
    emit imageLoaded(card, image);
}

void PictureLoaderWorker::cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl)
{
    redirectCache[originalUrl] = qMakePair(redirectUrl, QDateTime::currentDateTimeUtc());
    // saveRedirectCache();
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

void PictureLoaderWorker::clearNetworkCache()
{
    networkManager->cache()->clear();
}
