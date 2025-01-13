#include "picture_loader_worker.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_loader_worker_work.h"

#include <QDirIterator>
#include <QJsonDocument>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>
#include <utility>

// Card back returned by gatherer when card is not found
QStringList PictureLoaderWorker::md5Blacklist = QStringList() << "db0c48db407a907c16ade38de048a441";

PictureLoaderWorker::PictureLoaderWorker()
    : QObject(nullptr), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath()),
      picDownload(SettingsCache::instance().getPicDownload()), downloadRunning(false), loadQueueRunning(false),
      overrideAllCardArtWithPersonalPreference(SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference())
{
    connect(&SettingsCache::instance(), SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));
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

    cacheFilePath = SettingsCache::instance().getRedirectCachePath() + REDIRECT_CACHE_FILENAME;
    loadRedirectCache();
    cleanStaleEntries();

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this,
            &PictureLoaderWorker::saveRedirectCache);

    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);

    connect(&requestTimer, &QTimer::timeout, this, &PictureLoaderWorker::processQueuedRequests);
    requestTimer.setSingleShot(true);
}

PictureLoaderWorker::~PictureLoaderWorker()
{
    pictureLoaderThread->deleteLater();
}

QNetworkReply *PictureLoaderWorker::makeRequest(const QUrl &url, PictureLoaderWorkerWork *worker)
{
    constexpr int urlRequestDelayMs = 100; // Minimum delay between requests to the same URL (in milliseconds)

    // Check if we're rate-limited globally
    if (rateLimited) {
        requestQueue.append(qMakePair(url, worker));
        return nullptr;
    }

    // Check if a request was recently made to this URL
    QDateTime now = QDateTime::currentDateTimeUtc();
    if (lastRequestTime.contains(url)) {
        int elapsedMs = lastRequestTime[url].msecsTo(now);
        if (elapsedMs < urlRequestDelayMs) {
            // Queue the request if too soon to reissue
            requestQueue.append(qMakePair(url, worker));
            if (!requestTimer.isActive()) {
                requestTimer.start(urlRequestDelayMs);
            }
            return nullptr;
        }
    }

    // Update the last request time for this URL
    lastRequestTime[url] = now;

    // Check for cached redirects
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
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
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 429) {
            handleRateLimit(url, worker);
        } else {
            worker->picDownloadFinished(reply);
        }
        reply->deleteLater();
    });

    return reply;
}

void PictureLoaderWorker::handleRateLimit(const QUrl &url, PictureLoaderWorkerWork *worker)
{
    // Prevent multiple rate-limit handling
    if (!rateLimited) {
        int retryAfter = 70;
        rateLimited = true;
        qWarning() << "Rate limit exceeded! Queuing requests for" << retryAfter << "seconds.";

        // Start a timer to reset the rate-limited state
        rateLimitTimer.singleShot(retryAfter * 1000, this, [this]() {
            qWarning() << "Rate limit expired. Resuming queued requests.";
            processQueuedRequests();
        });
    }

    // Always queue the request even if already rate-limited
    requestQueue.append(qMakePair(url, worker));
}

void PictureLoaderWorker::processQueuedRequests()
{
    qWarning() << "Resuming queued requests";
    rateLimited = false;

    if (!requestQueue.isEmpty()) {
        // Process requests one by one, respecting the URL delay
        auto request = requestQueue.takeFirst();
        QDateTime now = QDateTime::currentDateTimeUtc();

        if (lastRequestTime.contains(request.first)) {
            int elapsedMs = lastRequestTime[request.first].msecsTo(now);
            if (elapsedMs < 100) {
                // Delay request if needed
                requestQueue.prepend(request);       // Put back in the queue
                requestTimer.start(100 - elapsedMs); // Schedule for the remaining time
                return;
            }
        }

        makeRequest(request.first, request.second);
    }
}

void PictureLoaderWorker::enqueueImageLoad(const CardInfoPtr &card)
{
    auto worker = new PictureLoaderWorkerWork(this, card);
    Q_UNUSED(worker);
}

void PictureLoaderWorker::imageLoadedSuccessfully(CardInfoPtr card, const QImage &image)
{
    emit imageLoaded(std::move(card), image);
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
