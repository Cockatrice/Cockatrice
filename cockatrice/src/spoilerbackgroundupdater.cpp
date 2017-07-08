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

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *apParent) : QObject(apParent)
{
    // User doesn't want to download spoilers... for now :)
    mbIsActiveThread = settingsCache->getDownloadSpoilersStatus();
    connect(settingsCache, SIGNAL(downloadSpoilerStatusChanged()), this, SLOT(changeActiveStatus()));

    // If there is a change in the settings, update the timer so we know when to update correctly
    connect(settingsCache, SIGNAL(downloadSpoilerTimeIndexChanged()), this, SLOT(handleNewTimeInterval()));

    if (! mbIsActiveThread)
    {
        return;
    }

    // User wants spoilers, so lets see if we're in spoiler season or not.
    downloadSpoilerSeasonStatusFileAndStartTimer();
}

SpoilerBackgroundUpdater::~SpoilerBackgroundUpdater()
{
    if (mpTimerForSpoilers)
    {
        delete mpTimerForSpoilers;
    }

    if (mpNetworkAccessManager)
    {
        delete mpNetworkAccessManager;
    }
}

void SpoilerBackgroundUpdater::changeActiveStatus()
{
    mbIsActiveThread = (settingsCache->getDownloadSpoilersStatus());
    qDebug() << "Spoiler Timer running status has been changed to" << mbIsActiveThread;

    // If they enable spoilers, auto-download the file (aka timer expired) and restart the timer
    if (mbIsActiveThread)
    {
        downloadSpoilerSeasonStatusFileAndStartTimer();
        if (mbIsActiveSpoilerSeason)
        {
            timeoutOccurredTimeToDownloadSpoilers();
        }
    }
}

void SpoilerBackgroundUpdater::timeoutOccurredTimeToDownloadSpoilers()
{
    qDebug() << "Spoiler Timer has finished";

    settingsCache->setDownloadSpoilerLastUpdateTime(QDateTime::currentMSecsSinceEpoch());

    // If this thread is active, then initiate download
    if (mbIsActiveThread)
    {
        qDebug() << "Timer thread is active, so attempting to download spoilers";
        downloadSpoilersFile(QString(GITHUB_SPOILERS_DOWNLOAD_URL));
    }

    // Now that we've downloaded the spoiler, reset the timer.
    // Take it from the top, Jenkins!
    runTimer(true);
}

void SpoilerBackgroundUpdater::handleNewTimeInterval()
{
    // Cause the timer to stop then restart w/ new interval
    qDebug() << "Spoiler Timer has new time interval established";
    runTimer(true);
}

void SpoilerBackgroundUpdater::runTimer(bool lbStopAndRestart)
{
    // If the timer doesn't exist (user ticked checkbox, for example, as an override, create it)
    if (mpTimerForSpoilers == nullptr)
    {
        // When timer ends, cause download of Spoilers XML file
        mpTimerForSpoilers = new QTimer(this);
        connect(mpTimerForSpoilers, SIGNAL(timeout()), this, SLOT(timeoutOccurredTimeToDownloadSpoilers()));
    }

    // Cause a restart of the timer as if we just restart via start() a timeout() will be
    // thrown which will trigger a download... which we don't want off time!
    if (lbStopAndRestart)
    {
        mpTimerForSpoilers->stop();
    }

    // Determine how long to wait between updates (Gets the # of hours from settingsCache / convert Mins -> Millis)
    int lnTimeToWaitBetween = settingsCache->getDownloadSpoilerTimeMinutes() * 60000;

    // How much time is needed until the next update (NOW() - Last Update Time == Time Remaining)
    long lnLastUpdateTime = settingsCache->getDownloadSpoilerLastUpdateTime();
    long lnTimeSinceLastUpdate = QDateTime::currentMSecsSinceEpoch() - lnLastUpdateTime;
    if (lnTimeSinceLastUpdate <= 500 || lnLastUpdateTime == -1)
    {
        // The update just occurred less then half a sec ago, so we will restart the timer from the beginning (time zero)
        mpTimerForSpoilers->start(lnTimeToWaitBetween);
    }
    else if (lnTimeSinceLastUpdate < lnTimeToWaitBetween)
    {
        // If we have checked for an update in the past and it's not yet time to check for another update
        // Start the clock to count down until lnTimeSinceLastUpdate
        mpTimerForSpoilers->start(static_cast<int>(lnTimeToWaitBetween - lnTimeSinceLastUpdate));
    }
    else
    {
        // It's been too long since we last checked (above threshold), so we will start now (time inf)
        mpTimerForSpoilers->start(0);
    }

    qDebug() << "Spoiler Timer has started and will finish in" << mpTimerForSpoilers->interval()/60000. << "minutes";
}

void SpoilerBackgroundUpdater::downloadSpoilersFile(QUrl acUrl)
{
    qDebug() << "Starting to download spoiler on NAM";
    if (mpNetworkAccessManager == nullptr)
    {
        mpNetworkAccessManager = new QNetworkAccessManager(this);
    }

    QNetworkReply *lpNetworkReply = mpNetworkAccessManager->get(QNetworkRequest(acUrl));

    connect(lpNetworkReply, SIGNAL(finished()), this, SLOT(actDownloadFinishedSpoilersFile()));
}

void SpoilerBackgroundUpdater::downloadSpoilerSeasonStatusFileAndStartTimer()
{
    qDebug() << "Checking spoiler season status";
    if (mpNetworkAccessManager == nullptr)
    {
        mpNetworkAccessManager = new QNetworkAccessManager(this);
    }

    QNetworkReply *lpNetworkReply = mpNetworkAccessManager->get(QNetworkRequest(QString(GITHUB_SPOILERS_STATUS_URL)));

    // Force the calling of actSeeIfSpoilerSeasonIsActive() in sequence instead of parallel
    // as we need to get that result before we start this timer thread up.
    connect(lpNetworkReply, SIGNAL(finished()), this, SLOT(actSeeIfSpoilerSeasonIsActive()));
}

void SpoilerBackgroundUpdater::actDownloadFinishedSpoilersFile()
{
    QNetworkReply *lpNetworkReply = static_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError leErrorCode = lpNetworkReply->error();
    QString lsSpoilerSavePath = settingsCache->getSpoilerDatabasePath();
    
    if (leErrorCode != QNetworkReply::NoError)
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Network error: %1.").arg(lpNetworkReply->errorString()));
        lpNetworkReply->deleteLater();
        return;
    }

    int lnStatusCode = lpNetworkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (lnStatusCode == 301 || lnStatusCode == 302)
    {
        // Was a redirect, will need to start over!
        QUrl lcRedirectUrl = lpNetworkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        downloadSpoilersFile(lcRedirectUrl);
    }
    else
    {
        // Read downloaded file into buffer
        mcDownloadedFileData = lpNetworkReply->readAll();

        if (saveSpoilersToFile(lsSpoilerSavePath))
        {
            // Reload the card database to import the spoilers
            QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
            QMessageBox::information(nullptr, tr("Success"), tr("Spoiler database updated and saved successfully to\n%1").arg(lsSpoilerSavePath));
        }
        else
        {
            QMessageBox::critical(nullptr, tr("Error"), tr("Spoiler database updated but could not be saved to %1").arg(lsSpoilerSavePath));
        }
    }

    lpNetworkReply->deleteLater();
}

void SpoilerBackgroundUpdater::actSeeIfSpoilerSeasonIsActive()
{
    QNetworkReply *lpNetworkReply = static_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError leErrorCode = lpNetworkReply->error();
    QString lsSpoilerSavePath = settingsCache->getSpoilerDatabasePath();

    if (leErrorCode != QNetworkReply::NoError)
    {
        // If there's a network issue
        mbIsActiveSpoilerSeason = false;
        QMessageBox::critical(nullptr, tr("Error"), tr("Network error: %1.").arg(lpNetworkReply->errorString()));
    }
    else
    {
        // If the file reads "enabled" then we're in spoiler season (true)
        // If the file reads anything else then we're NOT in spoiler season (false)
        QByteArray laFileData = lpNetworkReply->readAll();
        mbIsActiveSpoilerSeason = QString(laFileData).contains("enabled", Qt::CaseInsensitive);

        if (mbIsActiveSpoilerSeason)
        {
            // Start the timer for spoilers downloads
            runTimer();
        }
    }
}

bool SpoilerBackgroundUpdater::saveSpoilersToFile(const QString &asFileName)
{
    QFile lcFileToWriteTo(asFileName);
    bool lbRetVal = true;

    if (! lcFileToWriteTo.open(QIODevice::WriteOnly))
    {
        qDebug() << "File open (w) failed for" << asFileName;
        lbRetVal = false;
    }
    else if (lcFileToWriteTo.write(mcDownloadedFileData) == -1)
    {
        qDebug() << "File write (w) failed for" << asFileName;
        lbRetVal = false;
    }

    lcFileToWriteTo.close();
    return lbRetVal;
}