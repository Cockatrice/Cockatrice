#ifndef COCKATRICE_NEW_PICTURE_LOADER_WORKER_H
#define COCKATRICE_NEW_PICTURE_LOADER_WORKER_H

#include "../../../game/cards/card_database.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>

class NewPictureLoaderWorker : public QObject
{
    Q_OBJECT

public:
    explicit NewPictureLoaderWorker(QObject *parent, CardInfoPtr _cardInfoPtr, QNetworkRequest *_networkRequest);
    void doWork();

signals:
    void workFinishedSuccessfully(CardInfoPtr, QImage *);
    void workFinishedUnsuccessfully();
    void workFinished();

private:
    QNetworkAccessManager *networkManager;
    QNetworkRequest *networkRequest;
    CardInfoPtr cardInfoPtr;

    std::optional<QImage*> getImageFromReply(QNetworkReply *networkReply);
};

#endif // COCKATRICE_NEW_PICTURE_LOADER_WORKER_H
