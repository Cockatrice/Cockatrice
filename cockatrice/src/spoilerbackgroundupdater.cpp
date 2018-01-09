#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QtConcurrent>
#include <QCryptographicHash>

#include "spoilerbackgroundupdater.h"
#include "settingscache.h"
#include "carddatabase.h"
#include "main.h"
#include "window_main.h"

#define SPOILERS_STATUS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/SpoilerSeasonEnabled"
#define SPOILERS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *apParent) : QObject(apParent), cardUpdateProcess(nullptr)
{
    isSpoilerDownloadEnabled = settingsCache->getDownloadSpoilersStatus();
    if (isSpoilerDownloadEnabled)
    {
        // Start the process of checking if we're in spoiler season
        // File exists means we're in spoiler season
        startSpoilerDownloadProcess(SPOILERS_STATUS_URL, false);
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

    if (saveResults)
    {
        // This will write out to the file (used for spoiler.xml)
        connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinishedSpoilersFile()));
    }
    else
    {
        // This will check the status (used to see if we're in spoiler season or not)
        connect(reply, SIGNAL(finished()), this, SLOT(actCheckIfSpoilerSeasonEnabled()));
    }
}

void SpoilerBackgroundUpdater::actDownloadFinishedSpoilersFile()
{
    // Check for server reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError)
    {
        spoilerData = reply->readAll();
        reply->deleteLater();

        // Save the spoiler.xml file to the disk
        saveDownloadedFile(spoilerData);
    }
    else
    {
        qDebug() << "Error downloading spoilers file" << errorCode;
    }
}

void SpoilerBackgroundUpdater::actCheckIfSpoilerSeasonEnabled()
{
    auto *response = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = response->error();

    if (errorCode == QNetworkReply::ContentNotFoundError)
    {
        // Spoiler season is offline at this point, so the spoiler.xml file can be safely deleted
        // The user should run Oracle to get the latest card information
        QString fileName = settingsCache->getSpoilerCardDatabasePath();
        QFileInfo fi(fileName);
        QDir fileDir(fi.path());
        QFile file(fileName);

        // Delete the spoiler.xml file as we're not in spoiler season
        if (file.exists() && file.remove())
        {
            qDebug() << "Spoiler Season Offline, Deleting spoiler.xml";
        }

        /*
         * ALERT: Ensure two reloads of the card database do not happen
         * at the same time or a racetime condition can/will happen!
         */
        qDebug() << "Spoiler Season Offline, Reloading Database";
        QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
    }
    else if (errorCode == QNetworkReply::NoError)
    {
        qDebug() << "Spoiler Service Online";
        startSpoilerDownloadProcess(SPOILERS_URL, true);
    }
    else
    {
        qDebug() << "Error: Spoiler download failed with reason" << errorCode;
    }
}

bool SpoilerBackgroundUpdater::saveDownloadedFile(QByteArray data)
{
    QString fileName = settingsCache->getSpoilerCardDatabasePath();
    QFileInfo fi(fileName);
    QDir fileDir(fi.path());

    if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath()))
    {
        return false;
    }

    // Check if the data matches. If it does, then spoilers are up to date.
    if (getHash(fileName) == getHash(data))
    {
        if (trayIcon)
        {
            trayIcon->showMessage(tr("Spoilers already up to date"), tr("No new spoilers added"));
        }
        qDebug() << "Spoilers Up to Date, Reloading Database";
        QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Spoiler Service Error: File open (w) failed for" << fileName;
        return false;
    }

    if (file.write(data) == -1)
    {
        qDebug() << "Spoiler Service Error: File write (w) failed for" << fileName;
        return false;
    }

    file.close();

    /*
     * Data written, so reload the card database
     * ALERT: Ensure two reloads of the card database do not happen
     * at the same time or a racetime condition can/will happen!
     */
    qDebug() << "Spoiler Service Data Written, Reloading Database";
    QtConcurrent::run(db, &CardDatabase::loadCardDatabases);

    // If the user has notifications enabled, let them know
    // when the database was last updated
    if (trayIcon)
    {
        QList<QByteArray> lines = data.split('\n');

        foreach (QByteArray line, lines)
        {
            if (line.indexOf("created:") > -1)
            {
                QString timeStamp = QString(line).replace("created:", "").trimmed();
                trayIcon->showMessage(tr("Spoilers have been updated!"), tr("Last change:") + " " + timeStamp);
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

    if (file.open(QFile::ReadOnly))
    {
        // Only read the first 512 bytes (enough to get the "created" tag)
        const QByteArray bytes = file.read(512);

        QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
        hash.addData(bytes);

        file.close();
        return hash.result();
    }
    else
    {
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
    return hash.result();
}