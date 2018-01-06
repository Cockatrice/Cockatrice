#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QtConcurrent>
#include "spoilerbackgroundupdater.h"
#include "settingscache.h"
#include "carddatabase.h"
#include "main.h"
#include "window_main.h"

#define SPOILERS_STATUS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/SpoilerSeasonStatus"
#define SPOILERS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *apParent) : QObject(apParent), cardUpdateProcess(nullptr)
{
    isSpoilerDownloadEnabled = settingsCache->getDownloadSpoilersStatus();
    if (isSpoilerDownloadEnabled)
    {
        // Start the process of checking if we're in spoiler season
        // "enabled" means yes, anything else means no
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
}

void SpoilerBackgroundUpdater::actCheckIfSpoilerSeasonEnabled()
{
    // Check for server reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError)
    {
        spoilerData = reply->readAll();
        reply->deleteLater();

        isSpoilerSeason = (spoilerData.indexOf("enabled") > -1);

        // If it is spoiler season, go through the download process
        // of spoiler.xml from Cockatrice/Magic-Spoiler
        if (isSpoilerSeason)
        {
            qDebug() << "Spoiler Service Online";
            startSpoilerDownloadProcess(SPOILERS_URL, true);
        }
        else
        {
            qDebug() << "Spoiler Service Offline";
        }
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

    qDebug() << "Spoiler Service Data Written";
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
                trayIcon->showMessage(tr("Spoilers have been updated!"), timeStamp);
                return true;
            }
        }
    }

    return true;
}