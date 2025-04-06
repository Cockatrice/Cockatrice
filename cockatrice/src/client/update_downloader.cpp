#include "update_downloader.h"

#include <QDebug>
#include <QUrl>

UpdateDownloader::UpdateDownloader(QObject *parent) : QObject(parent), response(nullptr)
{
    netMan = new QNetworkAccessManager(this);
}

void UpdateDownloader::beginDownload(QUrl downloadUrl)
{
    // Save the original URL because we need it for the filename
    if (originalUrl.isEmpty())
        originalUrl = downloadUrl;

    response = netMan->get(QNetworkRequest(downloadUrl));
    connect(response, &QNetworkReply::finished, this, &UpdateDownloader::fileFinished);
    connect(response, &QNetworkReply::downloadProgress, this, &UpdateDownloader::downloadProgress);
    connect(this, &UpdateDownloader::stopDownload, response, &QNetworkReply::abort);
}

void UpdateDownloader::downloadError(QNetworkReply::NetworkError)
{
    if (response == nullptr)
        return;

    emit error(response->errorString().toUtf8());
}

void UpdateDownloader::fileFinished()
{
    // If we finished but there's a redirect, follow it
    QVariant redirect = response->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirect.isNull()) {
        beginDownload(redirect.toUrl());
        return;
    }

    // Handle any errors we had
    if (response->error()) {
        emit error(response->errorString());
        return;
    }

    // Work out the file name of the download
    QString fileName = QDir::temp().path() + QDir::separator() + originalUrl.toString().section('/', -1);

    // Save the build in a temporary directory
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error(tr("Could not open the file for reading."));
        return;
    }

    file.write(response->readAll());
    file.close();

    // Emit the success signal with a URL to the download file
    emit downloadSuccessful(QUrl::fromLocalFile(fileName));
}

void UpdateDownloader::downloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    emit progressMade(bytesRead, totalBytes);
}
