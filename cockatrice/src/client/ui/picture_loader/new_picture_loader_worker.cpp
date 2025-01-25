#include "new_picture_loader_worker.h"

#include <QBuffer>
#include <QImageReader>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>

NewPictureLoaderWorker::NewPictureLoaderWorker(QObject *parent,
                                               CardInfoPtr _cardInfoPtr,
                                               QNetworkRequest *_networkRequest)
    : QObject(parent), networkManager(new QNetworkAccessManager(this)), networkRequest(_networkRequest),
      cardInfoPtr(_cardInfoPtr)
{
    networkManager->setTransferTimeout();
}

void NewPictureLoaderWorker::doWork()
{
    qDebug() << "Starting Download for" << cardInfoPtr->getName() << networkRequest->url();
    auto *reply = networkManager->get(*networkRequest);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        std::optional<QImage*> image;

        if (reply->error() == QNetworkReply::NoError) {
            image = getImageFromReply(reply);
        }

        reply->deleteLater();
        if (image.has_value()) {
            qDebug() << "Download successful for" << cardInfoPtr->getName() << networkRequest->url();
            emit workFinishedSuccessfully(cardInfoPtr, image.value());
        } else {
            qDebug() << "Download failed for" << cardInfoPtr->getName() << networkRequest->url();
            emit workFinishedUnsuccessfully();
        }
        emit workFinished();
    });
}

std::optional<QImage*> NewPictureLoaderWorker::getImageFromReply(QNetworkReply *networkReply)
{
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(networkReply);

    static const int riffHeaderSize = 12; // RIFF_HEADER_SIZE from webp/format_constants.h
    const auto &replyHeader = networkReply->peek(riffHeaderSize);

    if (replyHeader.startsWith("RIFF") && replyHeader.endsWith("WEBP")) {
        auto imgBuf = QBuffer(this);
        imgBuf.setData(networkReply->readAll());

        auto movie = QMovie(&imgBuf);
        movie.start();
        movie.stop();

        return new QImage(movie.currentImage());
    }

    auto *testImage = new QImage();
    if (imgReader.read(testImage)) {
        return testImage;
    }

    return std::nullopt;
}