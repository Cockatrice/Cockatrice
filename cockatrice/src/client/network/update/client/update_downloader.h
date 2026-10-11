/**
 * @file update_downloader.h
 * @ingroup ClientUpdate
 */
//! \todo Document this file.

#ifndef COCKATRICE_UPDATEDOWNLOADER_H
#define COCKATRICE_UPDATEDOWNLOADER_H

#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtGlobal>
#include <qtmetamacros.h>

class QNetworkAccessManager;

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
