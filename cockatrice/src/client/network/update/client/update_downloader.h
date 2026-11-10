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
    // Starts a transfer and supersedes any reply still in flight: a cancelled or
    // failed download can be restarted on the same object, and late signals from
    // the superseded reply are dropped instead of reported.
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
    bool errorReported = false;
    bool cancelRequested = false;
private slots:
    void fileFinished();
    void downloadProgress(qint64 bytesRead, qint64 totalBytes);
    void downloadError(QNetworkReply::NetworkError);
    void cancelDownload();
};

#endif // COCKATRICE_UPDATEDOWNLOADER_H
