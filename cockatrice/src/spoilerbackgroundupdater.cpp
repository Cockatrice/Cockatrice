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

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *apParent) : QObject(apParent), cardUpdateProcess(nullptr)
{
    // If there is a change in the settings, update the timer so we know when to update correctly
    connect(settingsCache, SIGNAL(downloadSpoilerStatusChanged()), this, SLOT(changeActiveStatus()));
    connect(settingsCache, SIGNAL(downloadSpoilerTimeIndexChanged()), this, SLOT(handleNewTimeInterval()));

    qDebug() << "Spoiler Service online";

    runTimer();
    // TODO: User wants spoilers, so lets see if we're in spoiler season or not.
}

void SpoilerBackgroundUpdater::changeActiveStatus()
{
    mbIsActiveThread = (settingsCache->getDownloadSpoilersStatus());
    qDebug() << "Spoiler Timer running status has been changed to" << mbIsActiveThread;

    // If they enable spoilers, auto-download the file (aka timer expired) and restart the timer
    if (mbIsActiveThread)
    {
        timeoutOccurredTimeToDownloadSpoilers();
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
        downloadSpoilersFile();
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
        qDebug() << "Timer was null, established now";
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

/* CARD UPDATER
 * This was taken from window_main.cpp
 * The crossover was not working as expected so a copy was made here
 * If changes made there, they should probably be done here.
 * This is a refactor to do for sure
 */
void SpoilerBackgroundUpdater::downloadSpoilersFile()
{
    if (cardUpdateProcess)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("A card database update is already running."));
        return;
    }

    cardUpdateProcess = new QProcess(this);
    connect(cardUpdateProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(cardUpdateError(QProcess::ProcessError)));
    connect(cardUpdateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cardUpdateFinished(int, QProcess::ExitStatus)));

    // full "run the update" command; leave empty if not present
    QString updaterCmd;
    QString binaryName;
    QDir dir = QDir(QApplication::applicationDirPath());

#if defined(Q_OS_MAC)
    binaryName = getCardUpdaterBinaryName();

    // exit from the application bundle
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
    dir.cd(binaryName + ".app");
    dir.cd("Contents");
    dir.cd("MacOS");
#elif defined(Q_OS_WIN)
    binaryName = getCardUpdaterBinaryName() + ".exe";
#else
    binaryName = getCardUpdaterBinaryName();
#endif

    if (dir.exists(binaryName))
    {
        updaterCmd = dir.absoluteFilePath(binaryName);
    }

    if (updaterCmd.isEmpty())
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("Unable to run the card database updater: ") + dir.absoluteFilePath(binaryName));
        return;
    }

    cardUpdateProcess->start("\"" + updaterCmd + "\"");
}

void SpoilerBackgroundUpdater::cardUpdateError(QProcess::ProcessError err)
{
    QString error;
    switch(err)
    {
        case QProcess::FailedToStart:
            error = tr("failed to start.");
            break;
        case QProcess::Crashed:
            error = tr("crashed.");
            break;
        case QProcess::Timedout:
            error = tr("timed out.");
            break;
        case QProcess::WriteError:
            error = tr("write error.");
            break;
        case QProcess::ReadError:
            error = tr("read error.");
            break;
        case QProcess::UnknownError:
        default:
            error = tr("unknown error.");
            break;
    }

    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    QMessageBox::warning(nullptr, tr("Error"), tr("The card database updater exited with an error: %1").arg(error));
}

void SpoilerBackgroundUpdater::cardUpdateFinished(int, QProcess::ExitStatus)
{
    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    QMessageBox::information(nullptr, tr("Information"), tr("Update completed successfully.\nCockatrice will now reload the card database."));

    QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
}