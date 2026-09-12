#include "card_picture_loader_worker.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_loader_cache_method.h"
#include "card_picture_loader_local.h"
#include "card_picture_loader_worker_work.h"

#include <QDirIterator>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <utility>
#include <version_string.h>

static constexpr int MAX_REQUESTS_PER_SEC = DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT;
static constexpr int MIN_HOST_QUOTA = DownloadSettings::MIN_HOST_REQUEST_LIMIT;
static constexpr qint64 QUOTA_RECOVER_MS = 60000;       ///< Idle time before a reduced quota starts recovering
static constexpr int DISPATCH_INTERVAL_MS = 100;        ///< Pacing between individual network requests
static constexpr qint64 QUOTA_RESET_INTERVAL_MS = 1000; ///< Interval at which the request quota resets

CardPictureLoaderWorker::CardPictureLoaderWorker()
    : QObject(nullptr), picDownload(SettingsCache::instance().downloads().getPicDownload()),
      hostRequestLimits(SettingsCache::instance().downloads().getHostRequestLimits())
{
    networkManager = new QNetworkAccessManager(this);
    // We need a timeout to ensure requests don't hang indefinitely in case of
    // cache corruption, see related Qt bug: https://bugreports.qt.io/browse/QTBUG-111397
    // Use Qt's default timeout (30s, as of 2023-02-22)
    networkManager->setTransferTimeout();
    cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(SettingsCache::instance().getNetworkCachePath());
    cache->setMaximumCacheSize(1024L * 1024L *
                               static_cast<qint64>(SettingsCache::instance().cacheStorage().getNetworkCacheSizeInMB()));

    connect(&SettingsCache::instance().cacheStorage(), &CacheStorageSettings::networkCacheSizeChanged, cache,
            [this](int newSizeInMB) {
                if (cache) {
                    cache->setMaximumCacheSize(1024L * 1024L * static_cast<qint64>(newSizeInMB));
                }
            });

    networkManager->setCache(cache);

    // Use a ManualRedirectPolicy since we keep track of redirects in picDownloadFinished
    // We can't use NoLessSafeRedirectPolicy because it is not applied with AlwaysCache
    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    cacheFilePath = SettingsCache::instance().paths().getRedirectCachePath() + REDIRECT_CACHE_FILENAME;
    loadRedirectCache();
    cleanStaleEntries();

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this,
            &CardPictureLoaderWorker::saveRedirectCache);

    localLoader = new CardPictureLoaderLocal(this);

    pictureLoaderThread = new QThread;
    // The worker object frees itself once its thread finishes, so no event loop is left
    // running and the QThread is never destroyed while still executing.
    connect(pictureLoaderThread, &QThread::finished, this, &QObject::deleteLater);
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);

    // QTimer value members are not QObject children, so moveToThread on the worker doesn't move
    // them. They must live in the worker's thread to be started from the slot code that runs there.
    requestTimer.moveToThread(pictureLoaderThread);
    dispatchTimer.moveToThread(pictureLoaderThread);

    connect(this, &CardPictureLoaderWorker::imageLoadEnqueued, this, &CardPictureLoaderWorker::handleImageLoadEnqueued);

    connect(&requestTimer, &QTimer::timeout, this, &CardPictureLoaderWorker::resetRequestQuota);
    requestTimer.setInterval(static_cast<int>(QUOTA_RESET_INTERVAL_MS));

    connect(&dispatchTimer, &QTimer::timeout, this, &CardPictureLoaderWorker::dispatchQueuedRequest);
    dispatchTimer.setInterval(DISPATCH_INTERVAL_MS);

    connect(&SettingsCache::instance().downloads(), &DownloadSettings::hostRequestLimitsChanged, this,
            [this] { hostRequestLimits = SettingsCache::instance().downloads().getHostRequestLimits(); });
}

CardPictureLoaderWorker::~CardPictureLoaderWorker()
{
    saveRedirectCache();
}

void CardPictureLoaderWorker::shutdownThread()
{
    // The finished() -> deleteLater chain (wired in the constructor) frees this worker as soon as
    // its event loop exits, so nothing - not even a member read - may run once wait() returns.
    // QThread::quit() and QThread::wait() are thread-safe and may be called from the owning thread.
    QThread *thread = pictureLoaderThread;
    if (thread) {
        thread->quit();
        thread->wait();
    }
}

QThread *CardPictureLoaderWorker::workerThread() const
{
    return pictureLoaderThread;
}

bool CardPictureLoaderWorker::isRunning() const
{
    return pictureLoaderThread != nullptr && pictureLoaderThread->isRunning();
}

void CardPictureLoaderWorker::queueRequest(const QUrl &url, CardPictureLoaderWorkerWork *worker)
{
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        queueRequest(cachedRedirect, worker);
        return;
    }
    if (static_cast<CardPictureLoaderCacheMethod::CacheMethod>(
            SettingsCache::instance().cacheStorage().getCardPictureLoaderCacheMethod()) ==
            CardPictureLoaderCacheMethod::CacheMethod::NETWORK_CACHE &&
        cache->metaData(url).isValid()) {
        // A request that will be served from the disk cache never touches the network and therefore
        // doesn't use up any of the rate limit, so it gets to skip the queue.
        makeRequest(url, worker);
        return;
    }
    requestLoadQueue.append(qMakePair(url, worker));
    emit imageRequestQueued(url, worker->cardToDownload.getCard(), worker->cardToDownload.getSetName());
    processQueuedRequests();
}

QNetworkReply *CardPictureLoaderWorker::makeRequest(const QUrl &url, CardPictureLoaderWorkerWork *worker)
{
    // Check for cached redirects
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        emit imageRequestSucceeded(url);
        // The redirect target is a different host, which may itself be in 429 backoff; hand the
        // entry back to its worker so it waits the backoff out instead of dispatching straight
        // onto the backed-off host.
        if (CardPictureLoaderWorkerWork::rateLimiter().isRateLimited(cachedRedirect.host(),
                                                                     QDateTime::currentDateTime())) {
            worker->scheduleDeferredRetry();
            return nullptr;
        }
        return makeRequest(cachedRedirect, worker);
    }

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    req.setRawHeader("Accept", "image/avif,image/webp,image/apng,image/,/*;q=0.8");

    // Cached entries are served straight from the disk cache even when picture downloads are
    // enabled: re-fetching an already-cached image would burn the rate limit for nothing. Only a
    // genuine cache miss goes to the network, and only when downloads are enabled.
    bool useNetworkCache = static_cast<CardPictureLoaderCacheMethod::CacheMethod>(
                               SettingsCache::instance().cacheStorage().getCardPictureLoaderCacheMethod()) ==
                               CardPictureLoaderCacheMethod::CacheMethod::NETWORK_CACHE &&
                           (cache->metaData(url).isValid() || !picDownload);

    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                     useNetworkCache ? QNetworkRequest::AlwaysCache : QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply = networkManager->get(req);

    // Track in-flight replies per host so the unlocked fast path can bound how many requests it
    // issues at once, instead of creating replies that time out before Qt opens a connection.
    const QString host = url.host();
    hostInFlight.insert(host, hostInFlight.value(host) + 1);

    // Connect reply handling
    connect(reply, &QNetworkReply::finished, worker, [this, reply, worker, host] {
        hostInFlight.insert(host, qMax(0, hostInFlight.value(host) - 1));
        worker->handleNetworkReply(reply);
    });

    return reply;
}

void CardPictureLoaderWorker::resetRequestQuota()
{
    // Allowances are seeded lazily per host in processSingleRequest() when a request is first
    // looked at in a new second, so a host that enters the queue mid-second now gets its reduced
    // per-host allowance instead of falling through to the full per-second default.
    hostQuotaRemaining.clear();

    QDateTime now = QDateTime::currentDateTime();
    for (auto it = hostRequestQuota.begin(); it != hostRequestQuota.end();) {
        if (!hostLast429.contains(it.key()) || now.msecsTo(hostLast429.value(it.key())) < -QUOTA_RECOVER_MS) {
            if (hostAllowanceCeiling(it.key()) == DownloadSettings::UNLIMITED_HOST_QUOTA) {
                // A developer-unlocked host that fell back after a 429 recovers towards the default
                // allowance; once it gets there it becomes unlocked (fast-path) again.
                if (it.value() + 1 >= DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT) {
                    it = hostRequestQuota.erase(it);
                    continue;
                }
                it.value() += 1;
            } else {
                // Recover towards the host's effective allowance ceiling, which may be
                // lowered by the user's per-host request limits.
                it.value() = qMin(hostAllowanceCeiling(it.key()), it.value() + 1);
            }
        }
        ++it;
    }

    processQueuedRequests();
}

void CardPictureLoaderWorker::processQueuedRequests()
{
    Q_ASSERT(thread() == QThread::currentThread());

    if (requestLoadQueue.isEmpty()) {
        dispatchTimer.stop();
        requestTimer.stop();
        return;
    }
    // Start lazily from the worker's own thread: QTimer must be started in the thread it lives in.
    if (!requestTimer.isActive()) {
        requestTimer.start();
    }
    // Restarting an active timer would reset the pacing countdown, so a burst of enqueues could
    // keep starving the dispatcher; only start it when it has actually stopped.
    if (!dispatchTimer.isActive()) {
        dispatchTimer.start();
    }
}

void CardPictureLoaderWorker::dispatchQueuedRequest()
{
    if (requestLoadQueue.isEmpty()) {
        // All queued requests have been dispatched; stop the pacing and quota-reset timers.
        dispatchTimer.stop();
        requestTimer.stop();
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    bool dispatched = false;
    // Set while an unlocked host still has queued work blocked only by the in-flight cap; the
    // timer must keep running so it gets another try as soon as a slot frees. A host blocked by
    // its 429 backoff instead waits for the next quota-reset tick to restart the dispatcher.
    bool unlockedCapped = false;

    // Unlocked hosts (developer cap UNLIMITED_HOST_QUOTA) skip the pacing and the per-host
    // allowance: dispatch their queued requests back-to-back, bounded by their 429 backoff and the
    // per-host in-flight cap so a large burst can't queue replies that time out before Qt opens a
    // connection for them.
    for (int i = 0; i < requestLoadQueue.size();) {
        const auto &request = requestLoadQueue.at(i);
        const QString host = request.first.host();
        if (isUnlockedHost(host)) {
            if (CardPictureLoaderWorkerWork::rateLimiter().isRateLimited(host, now)) {
                ++i;
                continue;
            }
            if (hostInFlight.value(host) < MAX_IN_FLIGHT_PER_HOST) {
                makeRequest(request.first, request.second);
                requestLoadQueue.removeAt(i);
                dispatched = true;
                continue;
            }
            unlockedCapped = true;
        }
        ++i;
    }

    if (requestLoadQueue.isEmpty()) {
        dispatchTimer.stop();
        requestTimer.stop();
        return;
    }

    if (processSingleRequest()) {
        dispatched = true;
    }

    // Keep the timer running while there is progress to make or unlocked work waiting on a free
    // in-flight slot; otherwise no host has allowance left this second, so wait for the quota reset.
    if (!dispatched && !unlockedCapped) {
        dispatchTimer.stop();
    }
}

bool CardPictureLoaderWorker::processSingleRequest()
{
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < requestLoadQueue.size(); ++i) {
        const auto &request = requestLoadQueue.at(i);
        const QString host = request.first.host();
        // Don't dispatch requests to a host that is currently in its 429 backoff; hand the entry
        // back to its worker so it can wait the backoff out or fall through to another source,
        // instead of leaving it parked in the queue with no reply pending.
        if (CardPictureLoaderWorkerWork::rateLimiter().isRateLimited(host, now)) {
            requestLoadQueue.removeAt(i);
            request.second->startNextPicDownload();
            return true;
        }
        // Unlocked hosts are handled by dispatchQueuedRequest's fast path, bounded by the in-flight
        // cap; they must not fall through to the per-host allowance arithmetic below.
        if (isUnlockedHost(host)) {
            continue;
        }
        int ceiling = hostAllowanceCeiling(host);
        if (ceiling == DownloadSettings::UNLIMITED_HOST_QUOTA) {
            // A 429 dropped this unlocked host out of the fast path and installed a concrete
            // allowance; pace it against that allowance until the recovery loop unlocks it again.
            ceiling = hostRequestQuota.value(host, DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT);
        }
        // Seed the allowance lazily so a host that enters the queue mid-second gets its reduced
        // per-host allowance, clamped against the ceiling so a lowered user cap applies from this
        // second onward.
        if (!hostQuotaRemaining.contains(host)) {
            hostQuotaRemaining.insert(host, qMin(ceiling, hostRequestQuota.value(host, ceiling)));
        }
        int allowance = hostQuotaRemaining.value(host);
        if (allowance > 0) {
            hostQuotaRemaining.insert(host, allowance - 1);
            makeRequest(request.first, request.second);
            requestLoadQueue.removeAt(i);
            return true;
        }
    }
    return false;
}

int CardPictureLoaderWorker::hostAllowanceCeiling(const QString &host) const
{
    const int devCap = DownloadSettings::getDeveloperHostCaps().value(host, MAX_REQUESTS_PER_SEC);
    if (devCap == DownloadSettings::UNLIMITED_HOST_QUOTA && !hostRequestLimits.contains(host)) {
        return DownloadSettings::UNLIMITED_HOST_QUOTA;
    }
    const int requested = hostRequestLimits.value(host, devCap);
    return SettingsCache::instance().downloads().clampHostRequestLimit(host, requested);
}

bool CardPictureLoaderWorker::isUnlockedHost(const QString &host) const
{
    return hostAllowanceCeiling(host) == DownloadSettings::UNLIMITED_HOST_QUOTA && !hostRequestQuota.contains(host);
}

void CardPictureLoaderWorker::onHostRateLimited(const QString &host)
{
    const int ceiling = hostAllowanceCeiling(host);
    // An unlocked host has no per-host allowance to halve. Install one instead so it drops out of
    // the unlocked fast path and is paced like a throttled host; the recovery loop in
    // resetRequestQuota() then walks it back up and unlocks it again.
    const int base =
        ceiling == DownloadSettings::UNLIMITED_HOST_QUOTA ? DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT : ceiling;
    hostRequestQuota.insert(host, qMax(MIN_HOST_QUOTA, hostRequestQuota.value(host, base) / 2));
    hostLast429.insert(host, QDateTime::currentDateTime());
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
        if (it.value().second.addDays(SettingsCache::instance().cacheStorage().getRedirectCacheTtl()) < now) {
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
