#include "card_picture_loader_worker_work.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_loader_worker.h"

#include <QBuffer>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QMovie>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QThread>
#include <QThreadPool>
#include <QTimer>

ServerRateLimiter CardPictureLoaderWorkerWork::s_rateLimiter;

// Card back returned by gatherer when card is not found
static const QStringList MD5_BLACKLIST = {
    "db0c48db407a907c16ade38de048a441", // Old card back hash. Keep around just in case
    "fbc7d763c08771c260b39e2115414eeb"  // Current card back hash
};

CardPictureLoaderWorkerWork::CardPictureLoaderWorkerWork(const CardPictureLoaderWorker *worker, const ExactCard &toLoad)
    : QObject(nullptr), cardToDownload(CardPictureToLoad(toLoad)),
      picDownload(SettingsCache::instance().getPicDownload())
{
    // Hook up signals to the orchestrator
    connect(this, &CardPictureLoaderWorkerWork::requestImageDownload, worker, &CardPictureLoaderWorker::queueRequest);
    connect(this, &CardPictureLoaderWorkerWork::urlRedirected, worker, &CardPictureLoaderWorker::cacheRedirect);
    connect(this, &CardPictureLoaderWorkerWork::cachedUrlInvalidated, worker,
            &CardPictureLoaderWorker::removedCachedUrl);
    connect(this, &CardPictureLoaderWorkerWork::imageLoaded, worker, &CardPictureLoaderWorker::handleImageLoaded);
    connect(this, &CardPictureLoaderWorkerWork::requestSucceeded, worker,
            &CardPictureLoaderWorker::imageRequestSucceeded);
    connect(this, &CardPictureLoaderWorkerWork::rateLimited, worker, &CardPictureLoaderWorker::onHostRateLimited);

    // Hook up signals to settings
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    startNextPicDownload();
}

void CardPictureLoaderWorkerWork::startNextPicDownload()
{
    QDateTime now = QDateTime::currentDateTime();
    while (!cardToDownload.getCurrentUrl().isEmpty() &&
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
        scheduleDeferredRetry();
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
    QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    bool redirectFailure = false;
    if (redirectTarget.isValid()) {
        QUrl url = reply->request().url();
        QUrl redirectUrl = redirectTarget.toUrl();
        if (redirectUrl.isRelative()) {
            redirectUrl = url.resolved(redirectUrl);
        }
        if (url == redirectUrl) {
            qCWarning(CardPictureLoaderWorkerWorkLog) << "recursive redirect detected!";
            redirectFailure = true;
        } else {
            emit urlRedirected(url, redirectUrl);
        }
    }

    if (redirectFailure || reply->error()) {
        handleFailedReply(reply);
    } else {
        handleSuccessfulReply(reply);
        emit requestSucceeded(reply->url());
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

        if (s_rateLimiter.rounds(host) == 1) {
            qCWarning(CardPictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: Too many requests from " << host
                << ", backing off until " << backoffUntil.toString(Qt::ISODate) << ", retrying the same url";
            scheduleDeferredRetry();
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

            emit requestImageDownload(reply->url(), this);
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
        QUrl redirectUrl = reply->header(QNetworkRequest::LocationHeader).toUrl();
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

    QImage image = tryLoadImageFromReply(reply);

    if (image.isNull()) {
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Possible " << (isFromCache ? "cached" : "downloaded") << " picture at "
            << reply->url().toDisplayString() << " could not be loaded: " << reply->errorString();

        picDownloadFailed();
    } else {
        qCDebug(CardPictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Image successfully " << (isFromCache ? "loaded from cached" : "downloaded from") << " url "
            << reply->url().toDisplayString();

        concludeImageLoad(image);
    }
}

QImage CardPictureLoaderWorkerWork::tryLoadImageFromReply(QNetworkReply *reply)
{
    static constexpr int riffHeaderSize = 12; // RIFF_HEADER_SIZE from webp/format_constants.h
    auto replyHeader = reply->peek(riffHeaderSize);

    if (replyHeader.startsWith("RIFF") && replyHeader.endsWith("WEBP")) {
        auto imgBuf = QBuffer(this);
        imgBuf.setData(reply->readAll());

        auto movie = QMovie(&imgBuf);
        movie.start();
        movie.stop();

        return movie.currentImage();
    }

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(reply);

    return imgReader.read();
}

void CardPictureLoaderWorkerWork::scheduleDeferredRetry()
{
    QDateTime now = QDateTime::currentDateTime();

    // Prefer waiting on the current URL's server so we retry the same source.
    QString currentHost = QUrl(cardToDownload.getCurrentUrl()).host();
    QDateTime backoffUntil = s_rateLimiter.deadline(currentHost);
    if (!s_rateLimiter.isRateLimited(currentHost, now)) {
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

    QTimer::singleShot(waitMs, this, [this] {
        s_rateLimiter.clearExpired(QDateTime::currentDateTime());

        cardToDownload.resetIndices();
        startNextPicDownload();
    });
}

void CardPictureLoaderWorkerWork::concludeImageLoad(const QImage &image)
{
    emit imageLoaded(cardToDownload.getCard(), image);
    deleteLater();
}

void CardPictureLoaderWorkerWork::picDownloadChanged()
{
    picDownload = SettingsCache::instance().getPicDownload();
}
