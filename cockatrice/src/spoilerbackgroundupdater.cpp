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

#define SPOILERS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *apParent) : QObject(apParent), cardUpdateProcess(nullptr)
{
    // TODO: See if we're in spoilers season or not

    qDebug() << "Spoiler Service Online";

    isSpoilerDownloadEnabled = settingsCache->getDownloadSpoilersStatus();
    if (isSpoilerDownloadEnabled)
    {
        startSpoilerDownloadProcess();
    }

    qDebug() << "Spoiler Service Completed";
}

void SpoilerBackgroundUpdater::startSpoilerDownloadProcess()
{
    // Order of Operations: downloadFromURL -> actDownloadFinishedSpoilersFile -> saveDownloadedFile
    auto spoilerURL = QUrl(SPOILERS_URL);
    downloadFromURL(spoilerURL);
}

void SpoilerBackgroundUpdater::downloadFromURL(QUrl url)
{
    auto *nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(url));

    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinishedSpoilersFile()));
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
        saveDownloadedFile(spoilerData);
    }
}

bool SpoilerBackgroundUpdater::saveDownloadedFile(QByteArray data)
{
    QString fileName = settingsCache->getSpoilerCardDatabasePath();
    QString windowName = tr("Save spoiler database");
    QString fileType = tr("XML; card database (*.xml)");

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
    if (trayIcon)
    {
        QList<QByteArray> lines = data.split('\n');

        foreach (QByteArray line, lines)
        {
            if (line.indexOf("created:") > -1)
            {
                QString str = QString(line).replace("created:", "").trimmed();
                trayIcon->showMessage(tr("Spoilers last updated at"), str);
                return true;
            }
        }
    }

    return true;
}