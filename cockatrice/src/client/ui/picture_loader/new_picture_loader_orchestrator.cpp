#include "new_picture_loader_orchestrator.h"

#include "new_picture_loader_worker.h"
#include "picture_to_load.h"

#include <QImage>
#include <QNetworkRequest>
#include <QThread>
#include <QTimer>

NewPictureLoaderOrchestrator::NewPictureLoaderOrchestrator(const unsigned int _maxRequestsPerSecond, QObject *parent)
    : QObject(parent), maxRequestsPerSecond(_maxRequestsPerSecond)
{
    dispatchTimer = new QTimer(this);
    connect(dispatchTimer, &QTimer::timeout, this, &NewPictureLoaderOrchestrator::dequeueRequests);
    dispatchTimer->start(1000);
}

void NewPictureLoaderOrchestrator::enqueueImageLoad(CardInfoPtr card)
{
    const PictureToLoad cardToDownload(card);
    const QUrl cardImageUrl(cardToDownload.getCurrentUrl());
    auto *networkRequest = new QNetworkRequest(cardImageUrl);

    qDebug() << "Enqueued" << card->getName() << "for" << card->getPixmapCacheKey();
    requestsQueue.enqueue(std::make_pair<>(card, networkRequest));
}

void NewPictureLoaderOrchestrator::dequeueRequests()
{
    dispatchTimer->stop();

    for (unsigned int i = 0; i < maxRequestsPerSecond && !requestsQueue.isEmpty(); ++i) {
        const auto &cardInfoAndRequest = requestsQueue.dequeue();

        auto *worker = new NewPictureLoaderWorker(nullptr, cardInfoAndRequest.first, cardInfoAndRequest.second);

        auto *workerThread = new QThread;

        // Handle startup and cleanup for the worker thread
        connect(workerThread, &QThread::started, worker, &NewPictureLoaderWorker::doWork);
        connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

        // Kill the thread when it is done working (whether successful or not)
        connect(worker, &NewPictureLoaderWorker::workFinished, workerThread, &QThread::quit);

        // If the picture downloading was successful, cleanup the assets & load the image
        connect(worker, &NewPictureLoaderWorker::workFinishedSuccessfully, this, [=]() {
            delete cardInfoAndRequest.second;
        });
        connect(worker, &NewPictureLoaderWorker::workFinishedSuccessfully, this,
                &NewPictureLoaderOrchestrator::loadImage);

        // If the picture downloading was unsuccessful, re-enqueue the contents for later
        connect(worker, &NewPictureLoaderWorker::workFinishedUnsuccessfully, this, [=, this]() {
            qDebug() << "There was an error downloading" << cardInfoAndRequest.first->getName();
            requestsQueue.enqueue(cardInfoAndRequest);
        });

        worker->moveToThread(workerThread);
        workerThread->start(QThread::LowPriority);
    }

    dispatchTimer->start();
}
