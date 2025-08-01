#include "picture_loader_worker_work.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_loader_worker.h"

#include <QBuffer>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>

// Card back returned by gatherer when card is not found
static const QStringList MD5_BLACKLIST = {"db0c48db407a907c16ade38de048a441"};

PictureLoaderWorkerWork::PictureLoaderWorkerWork(const PictureLoaderWorker *worker, const ExactCard &toLoad)
    : QObject(nullptr), cardToDownload(PictureToLoad(toLoad)), picDownload(SettingsCache::instance().getPicDownload())
{
    // Hook up signals to the orchestrator
    connect(this, &PictureLoaderWorkerWork::requestImageDownload, worker, &PictureLoaderWorker::queueRequest);
    connect(this, &PictureLoaderWorkerWork::urlRedirected, worker, &PictureLoaderWorker::cacheRedirect);
    connect(this, &PictureLoaderWorkerWork::cachedUrlInvalidated, worker, &PictureLoaderWorker::removedCachedUrl);
    connect(this, &PictureLoaderWorkerWork::imageLoaded, worker, &PictureLoaderWorker::handleImageLoaded);
    connect(this, &PictureLoaderWorkerWork::requestSucceeded, worker, &PictureLoaderWorker::imageRequestSucceeded);

    // Hook up signals to settings
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    pictureLoaderThread = new QThread;
    moveToThread(pictureLoaderThread);

    connect(pictureLoaderThread, &QThread::started, this, &PictureLoaderWorkerWork::startNextPicDownload);

    // clean up threads once loading finishes
    connect(this, &QObject::destroyed, pictureLoaderThread, &QThread::quit);
    connect(pictureLoaderThread, &QThread::finished, pictureLoaderThread, &QObject::deleteLater);

    pictureLoaderThread->start(QThread::LowPriority);
}

void PictureLoaderWorkerWork::startNextPicDownload()
{
    QString picUrl = cardToDownload.getCurrentUrl();

    if (picUrl.isEmpty()) {
        picDownloadFailed();
    } else {
        QUrl url(picUrl);
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Trying to fetch picture from url "
            << url.toDisplayString();
        emit requestImageDownload(url, this);
    }
}

/**
 * Starts another pic download using the next possible url combination for the card.
 * If all possibilities are exhausted, then concludes the image loading with an empty QImage.
 */
void PictureLoaderWorkerWork::picDownloadFailed()
{
    /* Take advantage of short-circuiting here to call the nextUrl until one
       is not available.  Only once nextUrl evaluates to false will this move
       on to nextSet.  If the Urls for a particular card are empty, this will
       effectively go through the sets for that card. */
    if (cardToDownload.nextUrl() || cardToDownload.nextSet()) {
        startNextPicDownload();
    } else {
        qCWarning(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getInfo().getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Picture NOT found, "
            << (picDownload ? "download failed" : "downloads disabled")
            << ", no more url combinations to try: BAILING OUT";
        concludeImageLoad(QImage());
    }
}

/**
 *
 * @param reply The reply. Takes ownership of the object
 */
void PictureLoaderWorkerWork::handleNetworkReply(QNetworkReply *reply)
{
    QVariant redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirectTarget.isValid()) {
        QUrl url = reply->request().url();
        QUrl redirectUrl = redirectTarget.toUrl();
        if (redirectUrl.isRelative()) {
            redirectUrl = url.resolved(redirectUrl);
        }
        emit urlRedirected(url, redirectUrl);
    }

    if (reply->error()) {
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

void PictureLoaderWorkerWork::handleFailedReply(const QNetworkReply *reply)
{
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 429) {
        qCWarning(PictureLoaderWorkerWorkLog) << "Too many requests.";
    } else {
        bool isFromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

        if (isFromCache) {
            qCDebug(PictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: Removing corrupted cache file for url "
                << reply->url().toDisplayString() << " and retrying (" << reply->errorString() << ")";

            emit cachedUrlInvalidated(reply->url());

            emit requestImageDownload(reply->url(), this);
        } else {
            qCDebug(PictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard().getName()
                << " set: " << cardToDownload.getSetName() << "]: " << (picDownload ? "Download" : "Cache search")
                << " failed for url " << reply->url().toDisplayString() << " (" << reply->errorString() << ")";

            picDownloadFailed();
        }
    }
}

void PictureLoaderWorkerWork::handleSuccessfulReply(QNetworkReply *reply)
{
    bool isFromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

    // List of status codes from https://doc.qt.io/qt-6/qnetworkreply.html#redirected
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 ||
        statusCode == 308) {
        QUrl redirectUrl = reply->header(QNetworkRequest::LocationHeader).toUrl();
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: following " << (isFromCache ? "cached redirect" : "redirect") << " to "
            << redirectUrl.toDisplayString();
        emit requestImageDownload(redirectUrl, this);
        return;
    }

    // peek is used to keep the data in the buffer for use by QImageReader
    const QByteArray &picData = reply->peek(reply->size());

    if (imageIsBlackListed(picData)) {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Picture found, but blacklisted, will consider it as not found";

        picDownloadFailed();
        return;
    }

    QImage image = tryLoadImageFromReply(reply);

    if (image.isNull()) {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Possible " << (isFromCache ? "cached" : "downloaded") << " picture at "
            << reply->url().toDisplayString() << " could not be loaded: " << reply->errorString();

        picDownloadFailed();
    } else {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard().getName() << " set: " << cardToDownload.getSetName()
            << "]: Image successfully " << (isFromCache ? "loaded from cached" : "downloaded from") << " url "
            << reply->url().toDisplayString();

        concludeImageLoad(image);
    }
}

/**
 * @param reply The reply to load the image from
 * @return The loaded image, or an empty QImage if loading failed
 */
QImage PictureLoaderWorkerWork::tryLoadImageFromReply(QNetworkReply *reply)
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

/**
 * Call this method when the image has finished being loaded.
 * @param image The image that was loaded. Empty QImage indicates failure.
 */
void PictureLoaderWorkerWork::concludeImageLoad(const QImage &image)
{
    emit imageLoaded(cardToDownload.getCard(), image);
    deleteLater();
}

void PictureLoaderWorkerWork::picDownloadChanged()
{
    picDownload = SettingsCache::instance().getPicDownload();
}
