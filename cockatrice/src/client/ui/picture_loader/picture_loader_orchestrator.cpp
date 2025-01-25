#include "picture_loader_orchestrator.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_loader_worker.h"
#include "rate_limited_network_manager.h"

#include <QDirIterator>
#include <QJsonDocument>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>
#include <utility>

// Card back returned by gatherer when card is not found
QStringList PictureLoaderOrchestrator::md5Blacklist = QStringList() << "db0c48db407a907c16ade38de048a441";

/*
 * Generic idea:
 * - Orchestrator can fire off up to X threads per second, each which will run right away
 * - Orchestrator will keep a backlog of requests
 * -
 *
 */

PictureLoaderOrchestrator::PictureLoaderOrchestrator()
    : QObject(nullptr), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath()),
      picDownload(SettingsCache::instance().getPicDownload()), downloadRunning(false), loadQueueRunning(false)
{
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
            &PictureLoaderOrchestrator::saveRedirectCache);

    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);
}

PictureLoaderOrchestrator::~PictureLoaderOrchestrator()
{
    pictureLoaderThread->deleteLater();
}

QNetworkReply *PictureLoaderOrchestrator::makeRequest(const QUrl &url, PictureLoaderWorker *worker)
{
    if (rateLimited) {
        // Queue the request if currently rate-limited
        requestQueue.append(qMakePair(url, worker));
        return nullptr; // No immediate request
    }

    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        return makeRequest(cachedRedirect, worker);
    }

    QNetworkRequest req(url);
    if (!picDownload) {
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }

    QNetworkReply *reply = networkManager->get(req);

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
            handleRateLimit(reply, url, worker);
        } else {
            worker->picDownloadFinished(reply);
        }
        reply->deleteLater();
    });

    return reply;
}

void PictureLoaderOrchestrator::handleRateLimit(QNetworkReply *reply, const QUrl &url, PictureLoaderWorker *worker)
{
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj.value("object").toString() == "error" && jsonObj.value("code").toString() == "rate_limited") {
            int retryAfter = 70; // Default retry delay

            // Prevent multiple rate-limit handling
            if (!rateLimited) {
                rateLimited = true;
                qWarning() << "Scryfall rate limit hit! Queuing requests for" << retryAfter << "seconds.";

                // Start a timer to reset the rate-limited state
                rateLimitTimer.singleShot(retryAfter * 1000, this, [this]() {
                    qWarning() << "Rate limit expired. Resuming queued requests.";
                    processQueuedRequests();
                });
            }

            // Always queue the request even if already rate-limited
            requestQueue.append(qMakePair(url, worker));
        }
    }
}

void PictureLoaderOrchestrator::processQueuedRequests()
{
    qWarning() << "Resuming queued requests";
    rateLimited = false;

    while (!requestQueue.isEmpty()) {
        QPair<QUrl, PictureLoaderWorker *> request = requestQueue.takeFirst();
        makeRequest(request.first, request.second);
    }
}

void PictureLoaderOrchestrator::enqueueImageLoad(const CardInfoPtr &card)
{
    auto worker = new PictureLoaderWorker(this, card);
    Q_UNUSED(worker);
}

void PictureLoaderOrchestrator::imageLoadedSuccessfully(CardInfoPtr card, const QImage &image)
{
    emit imageLoaded(std::move(card), image);
}

void PictureLoaderOrchestrator::cacheRedirect(const QUrl &originalUrl, const QUrl &redirectUrl)
{
    redirectCache[originalUrl] = qMakePair(redirectUrl, QDateTime::currentDateTimeUtc());
    // saveRedirectCache();
}

QUrl PictureLoaderOrchestrator::getCachedRedirect(const QUrl &originalUrl) const
{
    if (redirectCache.contains(originalUrl)) {
        return redirectCache[originalUrl].first;
    }
    return {};
}

void PictureLoaderOrchestrator::loadRedirectCache()
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

void PictureLoaderOrchestrator::saveRedirectCache() const
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

void PictureLoaderOrchestrator::cleanStaleEntries()
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

void PictureLoaderOrchestrator::picDownloadChanged()
{
    QMutexLocker locker(&mutex);
    picDownload = SettingsCache::instance().getPicDownload();
}

void PictureLoaderOrchestrator::picsPathChanged()
{
    QMutexLocker locker(&mutex);
    picsPath = SettingsCache::instance().getPicsPath();
    customPicsPath = SettingsCache::instance().getCustomPicsPath();
}

void PictureLoaderOrchestrator::clearNetworkCache()
{
    networkManager->cache()->clear();
}