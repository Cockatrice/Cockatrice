#include "card_picture_loader_worker_work.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_loader_worker.h"

#include <QBuffer>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QThread>
#include <QThreadPool>
#include <libcockatrice/card/database/card_database_manager.h>

// Card back returned by gatherer when card is not found
static const QStringList MD5_BLACKLIST = {"db0c48db407a907c16ade38de048a441"};

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

    // Hook up signals to settings
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    startNextPicDownload();
}

void CardPictureLoaderWorkerWork::startNextPicDownload()
{
    QString picUrl = cardToDownload.getCurrentUrl();

    if (picUrl.isEmpty()) {
        picDownloadFailed();
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

void CardPictureLoaderWorkerWork::handleFailedReply(const QNetworkReply *reply)
{
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 429) {
        qCWarning(CardPictureLoaderWorkerWorkLog) << "Too many requests.";
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

void CardPictureLoaderWorkerWork::concludeImageLoad(const QImage &image)
{
    emit imageLoaded(cardToDownload.getCard(), image);
    deleteLater();
}

void CardPictureLoaderWorkerWork::picDownloadChanged()
{
    picDownload = SettingsCache::instance().getPicDownload();
}
