#include "picture_loader_worker.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_loader_worker_work.h"

#include <QDirIterator>
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
            &PictureLoaderWorker::saveRedirectCache);

    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);
}

PictureLoaderWorker::~PictureLoaderWorker()
{
    pictureLoaderThread->deleteLater();
}

QNetworkReply *PictureLoaderWorker::makeRequest(const QUrl &url, PictureLoaderWorkerWork *worker)
{
    // Check if the request leads to a cached redirect
    QUrl cachedRedirect = getCachedRedirect(url);
    if (!cachedRedirect.isEmpty()) {
        qCDebug(PictureLoaderWorkerLog).nospace()
            << "PictureLoader: [card: " << worker->cardToDownload.getCard()->getCorrectedName()
            << " set: " << worker->cardToDownload.getSetName() << "]: Using cached redirect for "
            << url.toDisplayString() << " to " << cachedRedirect.toDisplayString();
        return makeRequest(cachedRedirect, worker); // Use the cached redirect
    }

    // It's not cached so we actually have to make the request

    QNetworkRequest req(url);

    // If the user requests not to "download card images on the fly", we have to cache this network request

    if (!picDownload) {
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }

    // Make the actual network request

    QNetworkReply *reply = networkManager->get(req);

    // Connect an additional check to the finished signal to check if the request was redirected and if so, save it to
    // the redirection cache

    connect(reply, &QNetworkReply::finished, this, [this, reply, url, worker]() {
        QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if (redirectTarget.isValid()) {
            QUrl redirectUrl = redirectTarget.toUrl();
            if (redirectUrl.isRelative()) {
                redirectUrl = url.resolved(redirectUrl);
            }

            cacheRedirect(url, redirectUrl);
            qCDebug(PictureLoaderWorkerLog).nospace()
                << "PictureLoader: [card: " << worker->cardToDownload.getCard()->getCorrectedName()
                << " set: " << worker->cardToDownload.getSetName() << "]: Caching redirect from "
                << url.toDisplayString() << " to " << redirectUrl.toDisplayString();
        }
    });

    connect(
        reply, &QNetworkReply::finished, worker, [reply, worker]() { worker->picDownloadFinished(reply); },
        Qt::QueuedConnection);

    return reply;
}

void PictureLoaderWorker::enqueueImageLoad(const CardInfoPtr &card)
{
    auto worker = new PictureLoaderWorkerWork(this, card);
    Q_UNUSED(worker);
}

void PictureLoaderWorker::imageLoadedSuccessfully(CardInfoPtr card, const QImage &image)
{
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