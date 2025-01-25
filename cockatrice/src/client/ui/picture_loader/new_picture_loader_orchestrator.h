#ifndef COCKATRICE_NEW_PICTURE_LOADER_ORCHESTRATOR_H
#define COCKATRICE_NEW_PICTURE_LOADER_ORCHESTRATOR_H

#include "../../../game/cards/card_database.h"

#include <QObject>
#include <QQueue>

class QNetworkRequest;
class QTimer;

class QNetworkRequest;

class NewPictureLoaderOrchestrator : public QObject
{
    Q_OBJECT

public:
    explicit NewPictureLoaderOrchestrator(unsigned int _maxRequestsPerSecond, QObject *parent = nullptr);
    void enqueueImageLoad(CardInfoPtr card);

signals:
    void loadImage(CardInfoPtr cardInfoPtr, QImage *image);

private:
    unsigned int maxRequestsPerSecond;
    QQueue<QPair<CardInfoPtr, QNetworkRequest *>> requestsQueue;
    QTimer *dispatchTimer;

    void dequeueRequests();
};

#endif // COCKATRICE_NEW_PICTURE_LOADER_ORCHESTRATOR_H
