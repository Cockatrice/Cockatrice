#include <QUrl>

#include "update_downloader.h"

UpdateDownloader::UpdateDownloader(QObject *parent) : QObject(parent) {
    netMan = new QNetworkAccessManager(this);
}

void UpdateDownloader::beginDownload(QUrl downloadUrl) {
    //Save the original URL because we need it for the filename
    if (originalUrl.isEmpty())
        originalUrl = downloadUrl;

    response = netMan->get(QNetworkRequest(downloadUrl));
    connect(response, SIGNAL(finished()), this, SLOT(fileFinished()));
    connect(response, SIGNAL(readyRead()), this, SLOT(fileReadyRead()));
    connect(response, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    connect(response, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
}

void UpdateDownloader::downloadError(QNetworkReply::NetworkError) {
    emit error(response->errorString().toUtf8());
}

void UpdateDownloader::fileFinished() {
    //If we finished but there's a redirect, follow it
    QVariant redirect = response->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirect.isNull())
    {
        beginDownload(redirect.toUrl());
        return;
    }

    //Handle any errors we had
    if (response->error())
    {
        emit error(response->errorString());
        return;
    }

    //Work out the file name of the download
    QString fileName = QDir::temp().path() + QDir::separator() + originalUrl.toString().section('/', -1);

    //Save the build in a temporary directory
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error(tr("Could not open the file for reading."));
        return;
    }

    file.write(response->readAll());
    file.close();

    //Emit the success signal with a URL to the download file
    emit downloadSuccessful(QUrl::fromLocalFile(fileName));
}

void UpdateDownloader::downloadProgress(qint64 bytesRead, qint64 totalBytes) {
    emit progressMade(bytesRead, totalBytes);
}

