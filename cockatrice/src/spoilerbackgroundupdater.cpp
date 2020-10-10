#include "spoilerbackgroundupdater.h"

#include "carddatabase.h"
#include "main.h"
#include "settingscache.h"
#include "window_main.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QLocale>
#include <QMessageBox>
#include <QNetworkReply>
#include <QUrl>
#include <QtConcurrent>

#define SPOILERS_STATUS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/SpoilerSeasonEnabled"
#define SPOILERS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *apParent) : QObject(apParent), cardUpdateProcess(nullptr)
{
    isSpoilerDownloadEnabled = SettingsCache::instance().getDownloadSpoilersStatus();
    if (isSpoilerDownloadEnabled) {
        // Start the process of checking if we're in spoiler season
        // File exists means we're in spoiler season
        startSpoilerDownloadProcess(SPOILERS_STATUS_URL, false);
    } else {
        qDebug() << "Spoilers Disabled";
    }
}

void SpoilerBackgroundUpdater::startSpoilerDownloadProcess(QString url, bool saveResults)
{
    auto spoilerURL = QUrl(url);
    downloadFromURL(spoilerURL, saveResults);
}

void SpoilerBackgroundUpdater::downloadFromURL(QUrl url, bool saveResults)
{
    auto *nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(url));

    if (saveResults) {
        // This will write out to the file (used for spoiler.xml)
        connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinishedSpoilersFile()));
    } else {
        // This will check the status (used to see if we're in spoiler season or not)
        connect(reply, SIGNAL(finished()), this, SLOT(actCheckIfSpoilerSeasonEnabled()));
    }
}

void SpoilerBackgroundUpdater::actDownloadFinishedSpoilersFile()
{
    // Check for server reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError) {
        spoilerData = reply->readAll();

        // Save the spoiler.xml file to the disk
        saveDownloadedFile(spoilerData);

        reply->deleteLater();
        emit spoilerCheckerDone();
    } else {
        qDebug() << "Error downloading spoilers file" << errorCode;
        emit spoilerCheckerDone();
    }
}

bool SpoilerBackgroundUpdater::deleteSpoilerFile()
{
    QString fileName = SettingsCache::instance().getSpoilerCardDatabasePath();
    QFileInfo fi(fileName);
    QDir fileDir(fi.path());
    QFile file(fileName);

    // Delete the spoiler.xml file
    if (file.exists() && file.remove()) {
        qDebug() << "Deleting spoiler.xml";
        return true;
    }

    qDebug() << "Error: Spoiler.xml not found or not deleted";
    return false;
}

void SpoilerBackgroundUpdater::actCheckIfSpoilerSeasonEnabled()
{
    auto *response = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = response->error();

    if (errorCode == QNetworkReply::ContentNotFoundError) {
        // Spoiler season is offline at this point, so the spoiler.xml file can be safely deleted
        // The user should run Oracle to get the latest card information
        if (deleteSpoilerFile() && trayIcon) {
            trayIcon->showMessage(tr("Spoilers season has ended"), tr("Deleting spoiler.xml. Please run Oracle"));
        }

        qDebug() << "Spoiler Season Offline";
        emit spoilerCheckerDone();
    } else if (errorCode == QNetworkReply::NoError) {
        qDebug() << "Spoiler Service Online";
        startSpoilerDownloadProcess(SPOILERS_URL, true);
    } else if (errorCode == QNetworkReply::HostNotFoundError) {
        if (trayIcon) {
            trayIcon->showMessage(tr("Spoilers download failed"), tr("No internet connection"));
        }

        qDebug() << "Spoiler download failed due to no internet connection";
        emit spoilerCheckerDone();
    } else {
        if (trayIcon) {
            trayIcon->showMessage(tr("Spoilers download failed"), tr("Error") + " " + errorCode);
        }

        qDebug() << "Spoiler download failed with reason" << errorCode;
        emit spoilerCheckerDone();
    }
}

bool SpoilerBackgroundUpdater::saveDownloadedFile(QByteArray data)
{
    QString fileName = SettingsCache::instance().getSpoilerCardDatabasePath();
    QFileInfo fi(fileName);
    QDir fileDir(fi.path());

    if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
        return false;
    }

    // Check if the data matches. If it does, then spoilers are up to date.
    if (getHash(fileName) == getHash(data)) {
        if (trayIcon) {
            trayIcon->showMessage(tr("Spoilers already up to date"), tr("No new spoilers added"));
        }

        qDebug() << "Spoilers Up to Date";
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Spoiler Service Error: File open (w) failed for" << fileName;
        file.close();
        return false;
    }

    if (file.write(data) == -1) {
        qDebug() << "Spoiler Service Error: File write (w) failed for" << fileName;
        file.close();
        return false;
    }

    file.close();

    // Data written, so reload the card database
    qDebug() << "Spoiler Service Data Written";
    QtConcurrent::run(db, &CardDatabase::loadCardDatabases);

    // If the user has notifications enabled, let them know
    // when the database was last updated
    if (trayIcon) {
        QList<QByteArray> lines = data.split('\n');

        foreach (QByteArray line, lines) {
            if (line.contains("Created At:")) {
                QString timeStamp = QString(line).replace("Created At:", "").trimmed();
                timeStamp.chop(6); // Remove " (UTC)"

                auto utcTime = QLocale().toDateTime(timeStamp, "ddd, MMM dd yyyy, hh:mm:ss");
                utcTime.setTimeSpec(Qt::UTC);

                QString localTime = utcTime.toLocalTime().toString("MMM d, hh:mm");

                trayIcon->showMessage(tr("Spoilers have been updated!"), tr("Last change:") + " " + localTime);
                emit spoilersUpdatedSuccessfully();
                return true;
            }
        }
    }

    return true;
}

QByteArray SpoilerBackgroundUpdater::getHash(const QString fileName)
{
    QFile file(fileName);

    if (file.open(QFile::ReadOnly)) {
        // Only read the first 512 bytes (enough to get the "created" tag)
        const QByteArray bytes = file.read(512);

        QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
        hash.addData(bytes);

        qDebug() << "File Hash =" << hash.result();

        file.close();
        return hash.result();
    } else {
        qDebug() << "getHash ReadOnly failed!";
        file.close();
        return QByteArray();
    }
}

QByteArray SpoilerBackgroundUpdater::getHash(QByteArray data)
{
    // Only read the first 512 bytes (enough to get the "created" tag)
    const QByteArray bytes = data.left(512);

    QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
    hash.addData(bytes);

    qDebug() << "Data Hash =" << hash.result();

    return hash.result();
}
