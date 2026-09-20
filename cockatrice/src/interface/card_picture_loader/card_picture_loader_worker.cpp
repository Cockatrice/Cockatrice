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
    pictureLoaderThread->deleteLater();
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
        // The status bar still needs to reclaim this URL's widget even when we hand the request back
        // for a deferred retry instead of dispatching it onto the network.
        emit imageRequestSucceeded(url);
        // The redirect target is a different host, which may itself be in 429 backoff; hand the
        // entry back to its worker so it waits the backoff out instead of dispatching straight
        // onto the backed-off host.
        if (CardPictureLoaderWorkerWork::rateLimiter().isRateLimited(cachedRedirect.host(),
                                                                     QDateTime::currentDateTime())) {
            worker->scheduleDeferredRetry(cachedRedirect.host());
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
    bool useNetworkCache = !requestTouchesNetwork(url);

    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                     useNetworkCache ? QNetworkRequest::AlwaysCache : QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply = networkManager->get(req);

    // Track in-flight replies per host so the unlocked fast path can bound how many requests it
    // issues at once, instead of creating replies that time out before Qt opens a connection.
    const QString host = url.host();
    hostInFlight.insert(host, hostInFlight.value(host) + 1);

    // Release the in-flight slot when the reply is destroyed, not when it emits `finished`, and use
    // the worker (not the work object) as the context object: a reply can go away without ever
    // finishing (aborted, or a work object deleted while a reply is still pending), and a connection
    // bound to that work object's lifetime would then never run, permanently shrinking the fast
    // path's concurrency until it wedges. This way the slot is released exactly once.
    connect(reply, &QObject::destroyed, this,
            [this, host] { hostInFlight.insert(host, qMax(0, hostInFlight.value(host) - 1)); });

    // Connect reply handling; the work object is the context so its handler dies with it.
    connect(reply, &QNetworkReply::finished, worker, [worker, reply] { worker->handleNetworkReply(reply); });

    return reply;
}

void CardPictureLoaderWorker::resetRequestQuota()
{
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

    // Forget the per-second allowances; each host's allowance is re-seeded lazily from its
    // reduced sustained quota the first time it is dispatched in the new second, so a host that
    // enters the queue mid-second no longer falls through to a fresh full quota.
    hostQuotaRemaining.clear();

    updateTimerState();
}

void CardPictureLoaderWorker::processQueuedRequests()
{
    // QTimer must be started from the thread it lives in; if this public slot is ever reached from
    // another thread, replay it on the worker's event loop instead of letting start() fail silently.
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, &CardPictureLoaderWorker::processQueuedRequests, Qt::QueuedConnection);
        return;
    }
    updateTimerState();
}

void CardPictureLoaderWorker::dispatchQueuedRequest()
{
    if (requestLoadQueue.isEmpty()) {
        // All queued requests have been dispatched; stop the pacing timers.
        updateTimerState();
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
        // Dispatch decisions must key on the host the request will actually go to, not the URL that
        // merely redirects to it: a redirect learned after this URL was queued would otherwise
        // bypass the in-flight cap and drain the whole queue onto the target host unchecked.
        const QUrl resolvedUrl = resolveCachedRedirect(request.first);
        const QString host = resolvedUrl.host();
        if (isUnlockedHost(host)) {
            if (CardPictureLoaderWorkerWork::rateLimiter().isRateLimited(host, now)) {
                ++i;
                continue;
            }
            if (hostInFlight.value(host) < MAX_IN_FLIGHT_PER_HOST) {
                makeRequest(resolvedUrl, request.second);
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

void CardPictureLoaderWorker::updateTimerState()
{
    // Never restart an active timer: that would reset the pacing countdown and a burst of enqueues
    // could keep starving the dispatcher, so only (re)start a timer that has actually stopped.
    if (requestLoadQueue.isEmpty()) {
        dispatchTimer.stop();
        // Forget per-second allowances once nothing is pending: a stale zero would otherwise delay
        // the next single request by a full quota-reset interval.
        hostQuotaRemaining.clear();
    } else if (!dispatchTimer.isActive()) {
        dispatchTimer.start();
    }

    // The quota timer resets allowances every second and is also the only thing that heals a host
    // after a 429 (see resetRequestQuota). It must keep ticking while work is queued or a host is
    // still recovering below the ceiling, and only winds down once no host needs recovery anymore.
    // Keeping it alive during such idle periods lets reduced quotas recover as intended.
    bool hostRecovering = false;
    for (auto it = hostRequestQuota.cbegin(); it != hostRequestQuota.cend(); ++it) {
        if (it.value() < MAX_REQUESTS_PER_SEC) {
            hostRecovering = true;
            break;
        }
    }

    if (!requestLoadQueue.isEmpty() || hostRecovering) {
        if (!requestTimer.isActive()) {
            requestTimer.start();
        }
    } else if (requestTimer.isActive()) {
        requestTimer.stop();
    }
}

bool CardPictureLoaderWorker::processSingleRequest()
{
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < requestLoadQueue.size(); ++i) {
        // Copy the entry: takeAt(i) below erases within the list this reference points into.
        const auto request = requestLoadQueue.at(i);
        // Resolve cached redirects so the rate-limit and allowance arithmetic keys on the host the
        // request will actually hit (see resolveCachedRedirect).
        const QUrl resolvedUrl = resolveCachedRedirect(request.first);
        const QString host = resolvedUrl.host();
        // Don't dispatch requests to a host that is currently in its 429 backoff; hand the entry
        // back to its worker so it can wait the backoff out or fall through to another source,
        // instead of leaving it parked in the queue with no reply pending. Only applies to
        // requests that will actually touch the network: one that will be served from the disk
        // cache costs nothing and shouldn't wait out the 429.
        if (requestTouchesNetwork(request.first) &&
            CardPictureLoaderWorkerWork::rateLimiter().isRateLimited(host, now)) {
            // The queued URL is usually a cached-redirect target whose host differs from
            // cardToDownload.getCurrentUrl(), so scheduleDeferredRetry() (which waits out the
            // blocked host's deadline) is used instead of startNextPicDownload() looping on the
            // original host.
            auto entry = requestLoadQueue.takeAt(i);
            if (host != entry.first.host()) {
                // A cached redirect target is what is blocked, which the work object would not
                // discover from its own URL; wait out that specific host (with jitter) instead.
                entry.second->scheduleDeferredRetry(host);
            } else {
                entry.second->startNextPicDownload();
            }
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
            auto entry = requestLoadQueue.takeAt(i);
            makeRequest(resolvedUrl, entry.second);
            return true;
        }
    }
    return false;
}

bool CardPictureLoaderWorker::requestTouchesNetwork(const QUrl &url) const
{
    bool useNetworkCache = static_cast<CardPictureLoaderCacheMethod::CacheMethod>(
                               SettingsCache::instance().cacheStorage().getCardPictureLoaderCacheMethod()) ==
                               CardPictureLoaderCacheMethod::CacheMethod::NETWORK_CACHE &&
                           (cache->metaData(url).isValid() || !picDownload);
    return !useNetworkCache;
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

QUrl CardPictureLoaderWorker::resolveCachedRedirect(const QUrl &url) const
{
    // Follow the whole cached-redirect chain so dispatch keys on the host that is really hit. The
    // depth bound keeps a corrupt or self-referencing cache entry from spinning us forever.
    QUrl resolved = url;
    int depth = 0;
    while (depth++ < MAX_REDIRECT_CHAIN_DEPTH) {
        QUrl target = getCachedRedirect(resolved);
        if (target.isEmpty() || target == resolved) {
            break;
        }
        resolved = target;
    }
    return resolved;
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
