#include "update_downloader.h"

#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QUrl>

namespace
{
constexpr int DOWNLOAD_TIMEOUT_MS = 30000;
} // namespace

UpdateDownloader::UpdateDownloader(QObject *parent) : QObject(parent), response(nullptr)
{
    netMan = new QNetworkAccessManager(this);
}

void UpdateDownloader::beginDownload(QUrl downloadUrl)
{
    // Save the original URL because we need it for the filename
    if (originalUrl.isEmpty()) {
        originalUrl = downloadUrl;
    }

    if (response != nullptr) {
        response->abort();
        response->deleteLater();
    }

    errorReported = false;

    QNetworkRequest request(downloadUrl);
    // Without a timeout, a stalled connection leaves the dialog waiting forever.
    request.setTransferTimeout(DOWNLOAD_TIMEOUT_MS);

    response = netMan->get(request);
    connect(response, &QNetworkReply::finished, this, &UpdateDownloader::fileFinished);
    connect(response, &QNetworkReply::errorOccurred, this, &UpdateDownloader::downloadError);
    connect(response, &QNetworkReply::downloadProgress, this, &UpdateDownloader::downloadProgress);
    connect(this, &UpdateDownloader::stopDownload, response, &QNetworkReply::abort);
}

void UpdateDownloader::downloadError(QNetworkReply::NetworkError networkError)
{
    // A reply that was superseded by beginDownload() can still deliver a
    // queued error; it must not be reported against the current reply.
    if (response == nullptr || sender() != response || errorReported) {
        return;
    }

    // A user-requested abort is not an error worth showing.
    if (networkError == QNetworkReply::OperationCanceledError) {
        return;
    }

    errorReported = true;
    emit error(response->errorString());
}

void UpdateDownloader::fileFinished()
{
    // An aborted, superseded reply also emits finished(). Only the reply
    // currently stored in response may be consumed.
    if (response == nullptr || sender() != response) {
        return;
    }

    // errorOccurred normally reports this first; keep the check for errors that
    // only surface when the reply finishes.
    if (response->error() != QNetworkReply::NoError) {
        downloadError(response->error());
        response->deleteLater();
        response = nullptr;
        return;
    }

    // Give the update its own private directory. The name is unpredictable, so a
    // local user cannot pre-create or symlink the path, and the original file
    // name (including the installer suffix) is preserved inside it.
    QTemporaryDir downloadDir(QDir::temp().filePath(QStringLiteral("cockatrice-update-XXXXXX")));
    if (!downloadDir.isValid()) {
        emit error(tr("Could not create a temporary directory for the download."));
        response->deleteLater();
        response = nullptr;
        return;
    }
    downloadDir.setAutoRemove(false);

    const QString remoteName = QFileInfo(originalUrl.path()).fileName();
    const QString fileName = downloadDir.filePath(remoteName.isEmpty() ? QStringLiteral("update") : remoteName);

    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        downloadDir.remove();
        emit error(tr("Could not open the file for writing: %1").arg(file.errorString()));
        response->deleteLater();
        response = nullptr;
        return;
    }

    const QByteArray data = response->readAll();
    if (file.write(data) != data.size()) {
        file.cancelWriting();
        downloadDir.remove();
        emit error(tr("Could not write the downloaded file to disk."));
        response->deleteLater();
        response = nullptr;
        return;
    }

    if (!file.commit()) {
        downloadDir.remove();
        emit error(tr("Could not save the downloaded file: %1").arg(file.errorString()));
        response->deleteLater();
        response = nullptr;
        return;
    }

    response->deleteLater();
    response = nullptr;

    // Emit the success signal with a URL to the download file
    emit downloadSuccessful(QUrl::fromLocalFile(fileName));
}

void UpdateDownloader::downloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    emit progressMade(bytesRead, totalBytes);
}
