#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent>
#include "SpoilerBackgroundUpdaterThread.h"
#include "settingscache.h"
#include "carddatabase.h"
#include "main.h"

SpoilerBackgroundUpdaterThread::SpoilerBackgroundUpdaterThread(QObject *apParent) : QObject(apParent)
{
    // Determine if we're in spoilers season or not. If we're not, just kill this thread
    downloadSpoilerSeasonStatusFile();
    if (! mbIsActiveSpoilerSeason)
    {
        qDebug() << "Spoiler Timer disabled. NOT Spoilers Season";
        return;
    }

    // Determine if the thread should be active in downloading/updating spoilers
    mbIsActiveThread = settingsCache->getDownloadSpoilersStatus();
    connect(settingsCache, SIGNAL(downloadSpoilerStatusChanged()), this, SLOT(changeActiveStatus()));

    mpTimerForSpoilers = new QTimer(this);

    // When timer ends, cause download of Spoilers XML file
    connect(mpTimerForSpoilers, SIGNAL(timeout()), this, SLOT(timeoutOccurredTimeToDownloadSpoilers()));

    // If there is a change in the settings, update the timer so we know when to update correctly
    connect(settingsCache, SIGNAL(downloadSpoilerTimeIndexChanged()), this, SLOT(handleNewTimeInterval()));

    // Start the timer sequence and the domino effects
    runTimer();
}

SpoilerBackgroundUpdaterThread::~SpoilerBackgroundUpdaterThread()
{
    delete mpTimerForSpoilers;
}

void SpoilerBackgroundUpdaterThread::changeActiveStatus()
{
    mbIsActiveThread = (settingsCache->getDownloadSpoilersStatus());
    qDebug() << "Spoiler Timer running status has been changed to" << mbIsActiveThread;
}

void SpoilerBackgroundUpdaterThread::timeoutOccurredTimeToDownloadSpoilers()
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

void SpoilerBackgroundUpdaterThread::handleNewTimeInterval()
{
    // Cause the timer to stop then restart w/ new interval
    qDebug() << "Spoiler Timer has new time interval established";
    runTimer(true);
}

void SpoilerBackgroundUpdaterThread::runTimer(bool lbStopAndRestart)
{
    // Cause a restart of the timer as if we just restart via start() a timeout() will be
    // thrown which will trigger a download... which we don't want off time!
    if (lbStopAndRestart)
    {
        mpTimerForSpoilers->stop();
    }

    // Determine how long to wait between updates (Gets the # of hours from settingsCache / convert Mins -> Millis)
    float lnTimeToWaitBetween = settingsCache->getDownloadSpoilerTimeMinutes() * 60000;

    // How much time is needed until the next update (NOW() - Last Update Time == Time Remaining)
    long lnLastUpdateTime = settingsCache->getDownloadSpoilerLastUpdateTime();
    long lnTimeUntilNextUpdate = QDateTime::currentMSecsSinceEpoch() - lnLastUpdateTime;
    if (lnTimeUntilNextUpdate <= 500 || lnLastUpdateTime == -1)
    {
        // The update just occurred less then half a sec ago, so we will restart the timer from the beginning (time zero)
        mpTimerForSpoilers->start(static_cast<int>(lnTimeToWaitBetween));
    }
    else if (lnTimeUntilNextUpdate < lnTimeToWaitBetween)
    {
        // If we have checked for an update in the past and it's not yet time to check for another update
        // Start the clock to count down until lnTimeUntilNextUpdate
        mpTimerForSpoilers->start(static_cast<int>(lnTimeToWaitBetween - lnTimeUntilNextUpdate));
    }
    else
    {
        // It's been too long since we last checked (above threshold), so we will start now (time inf)
        mpTimerForSpoilers->start(0);
    }

    qDebug() << "Spoiler Timer has started and will finish in" << mpTimerForSpoilers->interval()/60000. << "minutes";
}

void SpoilerBackgroundUpdaterThread::downloadSpoilersFile(QUrl acUrl)
{
    qDebug() << "Starting to download spoiler on NAM";
    if (mpNetworkAccessManager == nullptr)
    {
        mpNetworkAccessManager = new QNetworkAccessManager(this);
    }

    QNetworkReply *lpNetworkReply = mpNetworkAccessManager->get(QNetworkRequest(acUrl));

    connect(lpNetworkReply, SIGNAL(finished()), this, SLOT(actDownloadFinishedTokensFile()));
}

void SpoilerBackgroundUpdaterThread::downloadSpoilerSeasonStatusFile()
{
    qDebug() << "Checking spoiler season status";
    if (mpNetworkAccessManager == nullptr)
    {
        mpNetworkAccessManager = new QNetworkAccessManager(this);
    }

    QNetworkReply *lpNetworkReply = mpNetworkAccessManager->get(QNetworkRequest(QString(GITHUB_SPOILERS_STATUS_URL)));

    // Force the calling of actSeeIfSpoilerSeasonIsActive() in sequence instead of parallel
    // as we need to get that result before we start this timer thread up.
    QEventLoop lcSubLoop;
    connect(lpNetworkReply, SIGNAL(finished()), this, SLOT(actSeeIfSpoilerSeasonIsActive()));
    connect(lpNetworkReply, SIGNAL(finished()), &lcSubLoop, SLOT(quit()));
    lcSubLoop.exec();
}

void SpoilerBackgroundUpdaterThread::actDownloadFinishedTokensFile()
{
    QNetworkReply *lpNetworkReply = static_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError leErrorCode = lpNetworkReply->error();
    QWidget *lpTempWidget = new QWidget();
    QString lsSpoilerSavePath = settingsCache->getSpoilerSavePath();

    // Center the popup(s) that are to come in the near future
    lpTempWidget->move(QApplication::desktop()->screen()->rect().center() - lpTempWidget->rect().center());
    
    if (leErrorCode != QNetworkReply::NoError)
    {
        QMessageBox::critical(lpTempWidget, tr("Error"), tr("Network error: %1.").arg(lpNetworkReply->errorString()));
    }
    else
    {
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
                QMessageBox::information(lpTempWidget, tr("Success"), tr("Spoiler database updated and saved successfully to\n%1").arg(lsSpoilerSavePath));
            }
            else
            {
                QMessageBox::critical(lpTempWidget, tr("Error"), tr("Spoiler database updated but could not be saved to %1").arg(lsSpoilerSavePath));
            }
        }
}

lpNetworkReply->deleteLater();
lpTempWidget->deleteLater();
}

void SpoilerBackgroundUpdaterThread::actSeeIfSpoilerSeasonIsActive()
{
    QNetworkReply *lpNetworkReply = static_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError leErrorCode = lpNetworkReply->error();
    QWidget *lpTempWidget = new QWidget();
    QString lsSpoilerSavePath = settingsCache->getSpoilerSavePath();

    // Center the popup(s) that are to come in the near future
    lpTempWidget->move(QApplication::desktop()->screen()->rect().center() - lpTempWidget->rect().center());

    if (leErrorCode != QNetworkReply::NoError)
    {
        // If there's a network issue
        mbIsActiveSpoilerSeason = false;
        QMessageBox::critical(lpTempWidget, tr("Error"), tr("Network error: %1.").arg(lpNetworkReply->errorString()));
    }
    else
    {
        // If the file reads "enabled" then we're in spoiler season (true)
        // If the file reads anything else then we're NOT in spoiler season (false)
        QByteArray laFileData = lpNetworkReply->readAll();
        mbIsActiveSpoilerSeason = QString(laFileData).contains("enabled", Qt::CaseInsensitive);
    }

    lpTempWidget->deleteLater();
}

bool SpoilerBackgroundUpdaterThread::saveSpoilersToFile(const QString &asFileName)
{
    QFile lcFileToWriteTo(asFileName);

    if (! lcFileToWriteTo.open(QIODevice::WriteOnly))
    {
        qDebug() << "File open (w) failed for" << asFileName;
        return false;
    }

    if (lcFileToWriteTo.write(mcDownloadedFileData) == -1)
    {
        qDebug() << "File write (w) failed for" << asFileName;
        return false;
    }

    lcFileToWriteTo.close();
    return true;
}