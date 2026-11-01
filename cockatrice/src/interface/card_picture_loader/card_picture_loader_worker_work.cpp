#include "card_picture_loader_worker_work.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_loader_image_formats.h"
#include "card_picture_loader_worker.h"
#include "card_picture_to_load.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/utility/server_rate_limiter.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QLoggingCategory>
#include <QMessageLogger>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QSharedPointer>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <limits>
#include <utility>

ServerRateLimiter CardPictureLoaderWorkerWork::s_rateLimiter;
#include <QtGlobal>
#include <libcockatrice/settings/download_settings.h>
#include <qnamespace.h>
#include <qobjectdefs.h>

// Card back returned by gatherer when card is not found
static const QStringList MD5_BLACKLIST = {
    "db0c48db407a907c16ade38de048a441", // Old card back hash. Keep around just in case
    "fbc7d763c08771c260b39e2115414eeb"  // Current card back hash
};

const ServerRateLimiter &CardPictureLoaderWorkerWork::rateLimiter()
{
    return s_rateLimiter;
}

CardPictureLoaderWorkerWork::CardPictureLoaderWorkerWork(CardPictureLoaderWorker *worker, const ExactCard &toLoad)
    : QObject(worker), cardToDownload(CardPictureToLoad(toLoad)),
      picDownload(SettingsCache::instance().downloads().getPicDownload())
{
    // Hook up signals to the orchestrator
    connect(this, &CardPictureLoaderWorkerWork::requestImageDownload, worker, &CardPictureLoaderWorker::queueRequest);
    connect(this, &CardPictureLoaderWorkerWork::urlRedirected, worker, &CardPictureLoaderWorker::cacheRedirect);
    connect(this, &CardPictureLoaderWorkerWork::redirectInvalidated, worker,
            &CardPictureLoaderWorker::invalidateRedirect);
    connect(this, &CardPictureLoaderWorkerWork::cachedUrlInvalidated, worker,
            &CardPictureLoaderWorker::removedCachedUrl);
    connect(this, &CardPictureLoaderWorkerWork::imageLoaded, worker, &CardPictureLoaderWorker::handleImageLoaded);
    connect(this, &CardPictureLoaderWorkerWork::rateLimited, worker, &CardPictureLoaderWorker::onHostRateLimited);

    // Hook up signals to settings
    connect(&SettingsCache::instance().downloads(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    deferredRetry = new QTimer(this);
    deferredRetry->setSingleShot(true);
    connect(deferredRetry, &QTimer::timeout, this, [this] {
        s_rateLimiter.clearExpired(QDateTime::currentDateTime());
        cardToDownload.resetIndices();
        startNextPicDownload();
    });

    startNextPicDownload();
}

void CardPictureLoaderWorkerWork::startNextPicDownload()
{
    redirectVisited.clear();
    QDateTime now = QDateTime::currentDateTime();
    while (picDownload && !cardToDownload.getCurrentUrl().isEmpty() &&
           s_rateLimiter.isRateLimited(QUrl(cardToDownload.getCurrentUrl()).host(), now)) {
        QString host = QUrl(cardToDownload.getCurrentUrl()).host();
        if (s_rateLimiter.rounds(host) == 1) {
            // First 429 round for this server: wait out the backoff and give it
            // one more chance instead of immediately falling through to a worse
            // source. A second 429 makes us fall through instead.
            qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
                << " set: " << cardToDownload.getSetName() << "]: Waiting out backoff for " << host << " to retry "
                << cardToDownload.getCurrentUrl();
            scheduleDeferredRetry();
            return;
        }

        // The server has already 429'd us at least twice, so further retries are
        // unlikely to succeed: move on to the other configured sources.
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Skipping rate-limited URL "
            << cardToDownload.getCurrentUrl() << " (server " << host << " still rate limiting)";
        if (!cardToDownload.nextUrl() && !cardToDownload.nextSet()) {
            scheduleDeferredRetry();
            return;
        }
    }

    QString picUrl = cardToDownload.getCurrentUrl();

    if (picUrl.isEmpty()) {
        if (picDownload) {
            scheduleDeferredRetry();
        } else {
            concludeImageLoad(QImage());
        }
    } else {
        QUrl url(picUrl);
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Trying to fetch picture from url "
            << url.toDisplayString();
        emit requestImageDownload(url, this);
    }
}

void CardPictureLoaderWorkerWork::picDownloadFailed()
{
    /* Take advantage of short-circuiting here to call the nextUrl until one
       is not available.  Only once nextUrl evaluates to false will this move
       on to nextSet.  If the Urls for a particular card are empty, this will
       effectively go through the sets for that card. */
    if (cardToDownload.nextUrl() || cardToDownload.nextSet()) {
        startNextPicDownload();
    } else {
        qCWarning(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Picture NOT found, "
            << (picDownload ? "download failed" : "downloads disabled")
            << ", no more url combinations to try: BAILING OUT";
        concludeImageLoad(QImage());
    }
}

void CardPictureLoaderWorkerWork::handleNetworkReply(QNetworkReply *reply)
{
    const QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool isRedirect = statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 ||
                            statusCode == 307 || statusCode == 308;
    bool redirectFailure = isRedirect && !redirectTarget.isValid();
    if (redirectTarget.isValid()) {
        const QUrl url = reply->url();
        const QUrl redirectUrl = url.resolved(redirectTarget.toUrl());
        redirectVisited.insert(url);
        if (!redirectUrl.isValid() || redirectUrl.isEmpty() || redirectVisited.contains(redirectUrl) ||
            redirectVisited.size() > 10) {
            qCWarning(CardPictureLoaderWorkerWorkLog) << "Invalid, cyclic or overlong redirect from" << url;
            redirectFailure = true;
        } else {
            emit urlRedirected(url, redirectUrl);
        }
    }

    if (redirectFailure) {
        for (const QUrl &visited : std::as_const(redirectVisited)) {
            emit redirectInvalidated(visited);
        }
        // Do not retry a cached cyclic redirect as a corrupt image: advance to
        // the next configured source, just as for an ordinary failed response.
        if (reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool()) {
            emit cachedUrlInvalidated(reply->url());
        }
        picDownloadFailed();
    } else if (reply->error()) {
        handleFailedReply(reply);
    } else {
        handleSuccessfulReply(reply);
    }

    reply->deleteLater();
}

static bool imageIsBlackListed(const QByteArray &picData)
{
    QString md5sum = QCryptographicHash::hash(picData, QCryptographicHash::Md5).toHex();
    return MD5_BLACKLIST.contains(md5sum);
}

void CardPictureLoaderWorkerWork::handleFailedReply(const QNetworkReply *reply)
{
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 429) {
        QString host = reply->url().host();
        QDateTime now = QDateTime::currentDateTime();

        qint64 retryAfterMs = 0;
        const QByteArray retryAfterHeader = reply->rawHeader("Retry-After");
        if (!retryAfterHeader.isEmpty()) {
            bool ok = false;
            int seconds = retryAfterHeader.toInt(&ok);
            if (ok && seconds > 0) {
                retryAfterMs = static_cast<qint64>(seconds) * 1000;
            } else {
                QDateTime retryAfterDate =
                    QDateTime::fromString(QString::fromLatin1(retryAfterHeader), Qt::RFC2822Date);
                if (retryAfterDate.isValid()) {
                    retryAfterMs = qMax<qint64>(0, now.msecsTo(retryAfterDate));
                }
            }
        }

        QDateTime backoffUntil = s_rateLimiter.on429(host, now, retryAfterMs);
        emit rateLimited(host);

        // Downloads may have been disabled while this request was in flight.
        // Record the server's limit for future online requests, but resume the
        // cache-only search now instead of scheduling a new network backoff.
        if (!picDownload) {
            cardToDownload.resetIndices();
            startNextPicDownload();
            return;
        }

        if (s_rateLimiter.rounds(host) == 1) {
            qCWarning(CardPictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: Too many requests from " << host
                << ", backing off until " << backoffUntil.toString(Qt::ISODate) << ", retrying the same url";
            scheduleDeferredRetry(host);
        } else {
            qCWarning(CardPictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: Too many requests from " << host
                << ", retry already attempted, falling through to other sources";
            picDownloadFailed();
        }
    } else {
        bool isFromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

        if (isFromCache) {
            qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: Removing corrupted cache file for url "
                << reply->url().toDisplayString() << " and retrying (" << reply->errorString() << ")";

            emit cachedUrlInvalidated(reply->url());

            if (!retryCachedUrl(reply->url())) {
                picDownloadFailed();
            }
        } else {
            qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: " << (picDownload ? "Download" : "Cache search")
                << " failed for url " << reply->url().toDisplayString() << " (" << reply->errorString() << ")";

            picDownloadFailed();
        }
    }
}

void CardPictureLoaderWorkerWork::handleSuccessfulReply(QNetworkReply *reply)
{
    bool isFromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

    // List of status codes from https://doc.qt.io/qt-6/qnetworkreply.html#redirected
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 ||
        statusCode == 308) {
        QUrl redirectUrl = reply->url().resolved(
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: following " << (isFromCache ? "cached redirect" : "redirect") << " to "
            << redirectUrl.toDisplayString();
        emit requestImageDownload(redirectUrl, this);
        return;
    }

    // A non-redirect successful response means the server is not rate limiting us anymore.
    s_rateLimiter.onSuccess(reply->url().host());

    // peek is used to keep the data in the buffer for use by QImageReader
    const QByteArray &picData = reply->peek(reply->size());

    if (imageIsBlackListed(picData)) {
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Picture found, but blacklisted, will consider it as not found";

        picDownloadFailed();
        return;
    }

    const auto decoded = CardPictureLoaderImageFormats::readImage(reply);
    const QImage &image = decoded.image;

    if (image.isNull()) {
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Possible " << (isFromCache ? "cached" : "downloaded") << " picture at "
            << reply->url().toDisplayString() << " could not be loaded: " << decoded.error;

        // HTTP success can still contain an invalid or unsupported image. Remove
        // that cache entry so future loads do not keep selecting the same bytes.
        emit cachedUrlInvalidated(reply->url());
        if (!isFromCache || !retryCachedUrl(reply->url())) {
            // Fresh bad responses advance to the next source; cached-only mode
            // must never turn a decoder failure into a network download.
            picDownloadFailed();
        }
    } else {
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Image successfully " << (isFromCache ? "loaded from cached" : "downloaded from") << " url "
            << reply->url().toDisplayString();

        concludeImageLoad(image);
    }
}

bool CardPictureLoaderWorkerWork::shouldBypassCache(const QUrl &url) const
{
    return picDownload && retriedCacheUrls.contains(url);
}

bool CardPictureLoaderWorkerWork::retryCachedUrl(const QUrl &url)
{
    if (!picDownload || retriedCacheUrls.contains(url)) {
        return false;
    }
    // Record before emitting: the queue can dispatch synchronously, and removing
    // an unreadable cache entry can fail. The retry must bypass that entry.
    retriedCacheUrls.insert(url);
    emit requestImageDownload(url, this);
    return true;
}

void CardPictureLoaderWorkerWork::scheduleDeferredRetry(const QString &preferredHost)
{
    QDateTime now = QDateTime::currentDateTime();

    // Prefer waiting on the server that is actually blocking the request: callers hand in the
    // rate-limited host when it differs from the current URL (e.g. a cached redirect target still
    // in backoff), otherwise fall back to the current URL's server so we retry the same source.
    QString waitHost = preferredHost.isEmpty() ? QUrl(cardToDownload.getCurrentUrl()).host() : preferredHost;
    QDateTime backoffUntil = s_rateLimiter.deadline(waitHost);
    if (!s_rateLimiter.isRateLimited(waitHost, now)) {
        backoffUntil = s_rateLimiter.earliestDeadline(now);
    }

    if (!backoffUntil.isValid()) {
        qCWarning(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: All URLs exhausted, no servers in backoff: BAILING OUT";
        concludeImageLoad(QImage());
        return;
    }

    qint64 waitMs = qMax<qint64>(0, now.msecsTo(backoffUntil));
    // Add some jitter to desynchronize concurrent retries and avoid a thundering herd.
    waitMs += QRandomGenerator::global()->bounded(5000);

    qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
        << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
        << " set: " << cardToDownload.getSetName() << "]: All URLs exhausted, scheduling deferred retry in " << waitMs
        << "ms";

    deferredRetry->start(static_cast<int>(qMin<qint64>(waitMs, std::numeric_limits<int>::max())));
}

void CardPictureLoaderWorkerWork::concludeImageLoad(const QImage &image)
{
    emit imageLoaded(cardToDownload.getCard(), image);
    deleteLater();
}

void CardPictureLoaderWorkerWork::picDownloadChanged()
{
    picDownload = SettingsCache::instance().downloads().getPicDownload();
    if (!picDownload && deferredRetry->isActive()) {
        deferredRetry->stop();
        cardToDownload.resetIndices();
        startNextPicDownload();
    }
}
