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

Q_LOGGING_CATEGORY(PictureLoaderWorkerWorkLog, "picture_loader.worker");

// Card back returned by gatherer when card is not found
QStringList PictureLoaderWorkerWork::md5Blacklist = QStringList() << "db0c48db407a907c16ade38de048a441";

PictureLoaderWorkerWork::PictureLoaderWorkerWork(PictureLoaderWorker *_worker, const CardInfoPtr &toLoad)
    : QThread(nullptr), worker(_worker), cardToDownload(toLoad)
{
    connect(this, &PictureLoaderWorkerWork::requestImageDownload, worker, &PictureLoaderWorker::makeRequest,
            Qt::QueuedConnection);
    connect(this, &PictureLoaderWorkerWork::imageLoaded, worker, &PictureLoaderWorker::imageLoadedSuccessfully,
            Qt::QueuedConnection);
    pictureLoaderThread = new QThread;
    pictureLoaderThread->start(QThread::LowPriority);
    moveToThread(pictureLoaderThread);
    startNextPicDownload();
}

PictureLoaderWorkerWork::~PictureLoaderWorkerWork()
{
    pictureLoaderThread->deleteLater();
}

bool PictureLoaderWorkerWork::cardImageExistsOnDisk(QString &setName, QString &correctedCardname)
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    QList<QString> picsPaths = QList<QString>();
    QDirIterator it(SettingsCache::instance().getCustomPicsPath(),
                    QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    // Recursively check all subdirectories of the CUSTOM folder
    while (it.hasNext()) {
        QString thisPath(it.next());
        QFileInfo thisFileInfo(thisPath);

        if (thisFileInfo.isFile() &&
            (thisFileInfo.fileName() == correctedCardname || thisFileInfo.completeBaseName() == correctedCardname ||
             thisFileInfo.baseName() == correctedCardname)) {
            picsPaths << thisPath; // Card found in the CUSTOM directory, somewhere
        }
    }

    if (!setName.isEmpty()) {
        picsPaths << SettingsCache::instance().getPicsPath() + "/" + setName + "/" + correctedCardname
                  // We no longer store downloaded images there, but don't just ignore
                  // stuff that old versions have put there.
                  << SettingsCache::instance().getPicsPath() + "/downloadedPics/" + setName + "/" + correctedCardname;
    }

    // Iterates through the list of paths, searching for images with the desired
    // name with any QImageReader-supported
    // extension
    for (const auto &_picsPath : picsPaths) {
        imgReader.setFileName(_picsPath);
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << correctedCardname << " set: " << setName << "]: Picture found on disk.";
            imageLoaded(cardToDownload.getCard(), image);
            return true;
        }
        imgReader.setFileName(_picsPath + ".full");
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderWorkerWorkLog).nospace() << "PictureLoader: [card: " << correctedCardname
                                                          << " set: " << setName << "]: Picture.full found on disk.";
            imageLoaded(cardToDownload.getCard(), image);
            return true;
        }
        imgReader.setFileName(_picsPath + ".xlhq");
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderWorkerWorkLog).nospace() << "PictureLoader: [card: " << correctedCardname
                                                          << " set: " << setName << "]: Picture.xlhq found on disk.";
            imageLoaded(cardToDownload.getCard(), image);
            return true;
        }
    }

    return false;
}

void PictureLoaderWorkerWork::startNextPicDownload()
{
    QString picUrl = cardToDownload.getCurrentUrl();

    if (picUrl.isEmpty()) {
        downloadRunning = false;
        picDownloadFailed();
    } else {
        QUrl url(picUrl);
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard()->getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Trying to fetch picture from url "
            << url.toDisplayString();
        emit requestImageDownload(url, this);
    }
}

void PictureLoaderWorkerWork::picDownloadFailed()
{
    /* Take advantage of short-circuiting here to call the nextUrl until one
       is not available.  Only once nextUrl evaluates to false will this move
       on to nextSet.  If the Urls for a particular card are empty, this will
       effectively go through the sets for that card. */
    if (cardToDownload.nextUrl() || cardToDownload.nextSet()) {
        startNextPicDownload();
    } else {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard()->getCorrectedName()
            << " set: " << cardToDownload.getSetName() << "]: Picture NOT found, "
            << (picDownload ? "download failed" : "downloads disabled")
            << ", no more url combinations to try: BAILING OUT";
        imageLoaded(cardToDownload.getCard(), QImage());
        cardToDownload.clear();
    }
    emit startLoadQueue();
}

void PictureLoaderWorkerWork::picDownloadFinished(QNetworkReply *reply)
{
    bool isFromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

    if (reply->error()) {
        if (isFromCache) {
            qCDebug(PictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard()->getName()
                << " set: " << cardToDownload.getSetName() << "]: Removing corrupted cache file for url "
                << reply->url().toDisplayString() << " and retrying (" << reply->errorString() << ")";

            networkManager->cache()->remove(reply->url());

            requestImageDownload(reply->url(), this);
        } else {
            qCDebug(PictureLoaderWorkerWorkLog).nospace()
                << "PictureLoader: [card: " << cardToDownload.getCard()->getName()
                << " set: " << cardToDownload.getSetName() << "]: " << (picDownload ? "Download" : "Cache search")
                << " failed for url " << reply->url().toDisplayString() << " (" << reply->errorString() << ")";

            picDownloadFailed();
            startNextPicDownload();
        }

        reply->deleteLater();
        return;
    }

    // List of status codes from https://doc.qt.io/qt-6/qnetworkreply.html#redirected
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 ||
        statusCode == 308) {
        QUrl redirectUrl = reply->header(QNetworkRequest::LocationHeader).toUrl();
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard()->getName()
            << " set: " << cardToDownload.getSetName() << "]: following "
            << (isFromCache ? "cached redirect" : "redirect") << " to " << redirectUrl.toDisplayString();
        requestImageDownload(redirectUrl, this);
        reply->deleteLater();
        return;
    }

    // peek is used to keep the data in the buffer for use by QImageReader
    const QByteArray &picData = reply->peek(reply->size());

    if (imageIsBlackListed(picData)) {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard()->getName()
            << " set: " << cardToDownload.getSetName()
            << "]: Picture found, but blacklisted, will consider it as not found";

        picDownloadFailed();
        reply->deleteLater();
        startNextPicDownload();
        return;
    }

    QImage testImage;

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(reply);

    bool logSuccessMessage = false;

    static const int riffHeaderSize = 12; // RIFF_HEADER_SIZE from webp/format_constants.h
    auto replyHeader = reply->peek(riffHeaderSize);

    if (replyHeader.startsWith("RIFF") && replyHeader.endsWith("WEBP")) {
        auto imgBuf = QBuffer(this);
        imgBuf.setData(reply->readAll());

        auto movie = QMovie(&imgBuf);
        movie.start();
        movie.stop();

        imageLoaded(cardToDownload.getCard(), movie.currentImage());
        logSuccessMessage = true;
    } else if (imgReader.read(&testImage)) {
        imageLoaded(cardToDownload.getCard(), testImage);
        logSuccessMessage = true;
    } else {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard()->getName()
            << " set: " << cardToDownload.getSetName() << "]: Possible " << (isFromCache ? "cached" : "downloaded")
            << " picture at " << reply->url().toDisplayString() << " could not be loaded: " << reply->errorString();

        picDownloadFailed();
    }

    if (logSuccessMessage) {
        qCDebug(PictureLoaderWorkerWorkLog).nospace()
            << "PictureLoader: [card: " << cardToDownload.getCard()->getName()
            << " set: " << cardToDownload.getSetName() << "]: Image successfully "
            << (isFromCache ? "loaded from cached" : "downloaded from") << " url " << reply->url().toDisplayString();
    } else {
        startNextPicDownload();
    }

    reply->deleteLater();
}

bool PictureLoaderWorkerWork::imageIsBlackListed(const QByteArray &picData)
{
    QString md5sum = QCryptographicHash::hash(picData, QCryptographicHash::Md5).toHex();
    return md5Blacklist.contains(md5sum);
}