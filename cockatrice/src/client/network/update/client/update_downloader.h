/**
 * @file update_downloader.h
 * @ingroup ClientUpdate
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_UPDATEDOWNLOADER_H
#define COCKATRICE_UPDATEDOWNLOADER_H

#include <QObject>
#include <QtNetwork>

class UpdateDownloader : public QObject
{
    Q_OBJECT
public:
    explicit UpdateDownloader(QObject *parent);
    void beginDownload(QUrl url);
signals:
    void downloadSuccessful(QUrl filepath);
    void progressMade(qint64 bytesRead, qint64 totalBytes);
    void error(QString errorString);
    void stopDownload();

private:
    QUrl originalUrl;
    QNetworkAccessManager *netMan;
    QNetworkReply *response;
private slots:
    void fileFinished();
    void downloadProgress(qint64 bytesRead, qint64 totalBytes);
    void downloadError(QNetworkReply::NetworkError);
};

#endif // COCKATRICE_UPDATEDOWNLOADER_H
