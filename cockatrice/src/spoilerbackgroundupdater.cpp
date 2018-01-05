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
    qDebug() << "Spoiler Service Online";

    isSpoilerDownloadEnabled = settingsCache->getDownloadSpoilersStatus();
    if (isSpoilerDownloadEnabled)
    {
        downloadSpoilersFile();
    }

    qDebug() << "Spoiler Service Completed";

    // TODO: User wants spoilers, so lets see if we're in spoiler season or not.
}

/* CARD UPDATER
 * This was taken from window_main.cpp
 * And then modified slightly.
 * TODO: Refactor
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

    dir.cd("/Users/zahalpern/Desktop/Stuff/Cockatrice/cockatrice/build/oracle/oracle.app/Contents/MacOS");
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

    cardUpdateProcess->start(updaterCmd, QStringList("-s"));
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