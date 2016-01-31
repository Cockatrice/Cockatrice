//
// Created by miguel on 28/12/15.
//

#ifndef COCKATRICE_UPDATEDOWNLOADER_H
#define COCKATRICE_UPDATEDOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QDate>
#include <QtNetwork>

class UpdateDownloader : public QObject {
Q_OBJECT
public:
    UpdateDownloader(QObject *parent);
    void beginDownload(QUrl url);
signals:
    void downloadSuccessful(QUrl filepath);
    void progressMade(qint64 bytesRead, qint64 totalBytes);
    void error(QString errorString);
private:
    QUrl originalUrl;
    QNetworkAccessManager *netMan;
    QNetworkReply *response;
private slots:
    void fileFinished();
    void downloadProgress(qint64 bytesRead, qint64 totalBytes);
    void downloadError(QNetworkReply::NetworkError);
};


#endif //COCKATRICE_UPDATEDOWNLOADER_H
