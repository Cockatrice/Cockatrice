#include "dlg_update.h"

#include "../../../client/settings/cache_settings.h"
#include "../client/network/update/client/client_update_checker.h"
#include "../client/network/update/client/release_channel.h"
#include "../interface/window_main.h"
#include "update/client/update_downloader.h"

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QList>
#include <QLocale>
#include <QMessageBox>
#include <QMessageLogger>
#include <QObject>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QSslSocket>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>
#include <version_string.h>

// Executable that, when it sits next to the downloaded update installer, is installed instead of
// it. A packager shipping a custom build - or someone testing one - can drop the file there and
// have Cockatrice run it rather than the official installer, without the release channel having to
// host an installer for that build. It is a drop-in and is run with the same arguments.
static const QString UPDATE_INSTALLER_OVERRIDE = "Cockatrice-Update-Override.exe";

DlgUpdate::DlgUpdate(QWidget *parent) : QDialog(parent)
{

    // Handle layout
    statusLabel = new QLabel(this);
    statusLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    statusLabel->setWordWrap(true);
    descriptionLabel =
        new QLabel(tr("Current release channel") +
                       QString(": %1").arg(tr(SettingsCache::instance().getUpdateReleaseChannel()->getName().toUtf8())),
                   this);
    progress = new QProgressBar(this);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setFixedWidth(350);

    ok = new QPushButton("Close", this);
    manualDownload = new QPushButton(tr("Reinstall"), this);
    stopDownload = new QPushButton(tr("Cancel Download"), this);
    gotoDownload = new QPushButton(tr("Open Download Page"), this);

    addStopDownloadAndRemoveOthers(false); // Add all buttons to box
    enableUpdateButton(false);             // Unless we know there's an update available, you can't install
    buttonBox->addButton(ok, QDialogButtonBox::AcceptRole);

    connect(gotoDownload, &QPushButton::clicked, this, &DlgUpdate::gotoDownloadPage);
    //! \todo Make reinstall button actually do something when clicked.
    // connect(manualDownload, &QPushButton::clicked, this, &DlgUpdate::downloadUpdate);
    connect(stopDownload, &QPushButton::clicked, this, &DlgUpdate::cancelDownload);
    connect(ok, &QPushButton::clicked, this, &DlgUpdate::closeDialog);

    auto *parentLayout = new QVBoxLayout(this);
    parentLayout->addWidget(descriptionLabel);
    parentLayout->addWidget(statusLabel);
    parentLayout->addWidget(progress);
    parentLayout->addWidget(buttonBox);

    setLayout(parentLayout);
    setWindowTitle(tr("Check for Client Updates"));

    setFixedHeight(this->sizeHint().height());
    setFixedWidth(this->sizeHint().width());

    // Check for SSL (this probably isn't necessary)
    if (!QSslSocket::supportsSsl()) {
        enableUpdateButton(false);
        QMessageBox::critical(this, tr("Error"),
                              tr("Cockatrice was not built with SSL support, therefore you cannot download updates "
                                 "automatically! \nPlease visit the download page to update manually."));
    }

    // Initialize the checker and downloader class
    uDownloader = new UpdateDownloader(this);
    connect(uDownloader, &UpdateDownloader::downloadSuccessful, this, &DlgUpdate::downloadSuccessful);
    connect(uDownloader, &UpdateDownloader::progressMade, this, &DlgUpdate::downloadProgressMade);
    connect(uDownloader, &UpdateDownloader::error, this, &DlgUpdate::downloadError);

    // Check for updates
    beginUpdateCheck();
}

void DlgUpdate::closeDialog()
{
    accept();
}

void DlgUpdate::gotoDownloadPage()
{
    QDesktopServices::openUrl(SettingsCache::instance().getUpdateReleaseChannel()->getManualDownloadUrl());
}

void DlgUpdate::downloadUpdate(const QString &releaseName)
{
    setLabel(tr("Downloading update: %1").arg(releaseName));
    addStopDownloadAndRemoveOthers(true); // Will remove all other buttons
    uDownloader->beginDownload(updateUrl);
}

void DlgUpdate::cancelDownload()
{
    emit uDownloader->stopDownload();
    setLabel("Download canceled");
    addStopDownloadAndRemoveOthers(false);
    downloadProgressMade(0, 1);
}

void DlgUpdate::beginUpdateCheck()
{
    progress->setMinimum(0);
    progress->setMaximum(0);
    setLabel(tr("Checking for updates..."));

    auto checker = new ClientUpdateChecker(this);
    connect(checker, &ClientUpdateChecker::finishedCheck, this, &DlgUpdate::finishedUpdateCheck);
    connect(checker, &ClientUpdateChecker::error, this, &DlgUpdate::updateCheckError);
    checker->check();
}

void DlgUpdate::finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *release)
{

    QString publishDate, versionName;

    // Update the UI to say we've finished
    progress->setMaximum(100);
    setLabel(tr("Finished checking for updates"));

    // If there are no available builds, then they can't auto update.
    enableUpdateButton(isCompatible);

    if (isCompatible) {
        // If there is an update, save its URL and work out its name
        updateUrl = release->getDownloadUrl();
    }

    // Give the user the appropriate message
    if (!needToUpdate) {
        // If there's no need to update, tell them that. However we still allow them to run the
        // downloader themselves if there's a compatible build
        QMessageBox::information(
            window(), tr("No Update Available"),
            tr("Cockatrice is up to date!") + "<br><br>" +
                tr("You are already running the latest version available in the chosen release channel.") + "<br>" +
                "<b>" + tr("Current version") + QString(":</b> %1<br>").arg(VERSION_STRING) + "<b>" +
                tr("Selected release channel") +
                QString(":</b> %1").arg(tr(SettingsCache::instance().getUpdateReleaseChannel()->getName().toUtf8())));
        return;
    }

    publishDate = release->getPublishDate().toString(QLocale().dateFormat(QLocale::LongFormat));
    if (isCompatible) {
        const QString message = tr("A new version of Cockatrice is available!") + "<br><br>" + "<b>" +
                                tr("New version") + QString(":</b> %1<br>").arg(release->getName()) + "<b>" +
                                tr("Released") +
                                QString(":</b> %1 (<a href=\"%2\">").arg(publishDate, release->getDescriptionUrl()) +
                                tr("Changelog") + "</a>)<br><br>" + tr("Do you want to update now?");

        if (QtUtils::confirm(window(), tr("Update Available"), message)) {
            downloadUpdate(release->getName());
        } else {
            closeDialog();
        }
    } else {
        QMessageBox::information(
            window(), tr("Update Available"),
            tr("A new version of Cockatrice is available!") + "<br><br>" + "<b>" + tr("New version") +
                QString(":</b> %1<br>").arg(release->getName()) + "<b>" + tr("Released") +
                QString(":</b> %1 (<a href=\"%2\">").arg(publishDate, release->getDescriptionUrl()) + tr("Changelog") +
                "</a>)<br><br>" +
                tr("Unfortunately, the automatic updater failed to find a compatible download. \nYou may have to "
                   "manually download the new version.") +
                "<br><br>" +
                tr("Please check the <a href=\"%1\">releases page</a> on our Github and download the build for your "
                   "system.")
                    .arg(release->getDescriptionUrl()));
    }
}

void DlgUpdate::enableUpdateButton(bool enable)
{
    manualDownload->setEnabled(enable);
}

void DlgUpdate::addStopDownloadAndRemoveOthers(bool enable)
{
    if (enable) {
        buttonBox->addButton(stopDownload, QDialogButtonBox::ActionRole);
        buttonBox->removeButton(manualDownload);
        buttonBox->removeButton(gotoDownload);
    } else {
        buttonBox->removeButton(stopDownload);
        buttonBox->addButton(manualDownload, QDialogButtonBox::ActionRole);
        buttonBox->addButton(gotoDownload, QDialogButtonBox::ActionRole);
    }
}

void DlgUpdate::enableOkButton(bool enable)
{
    ok->setEnabled(enable);
}

void DlgUpdate::setLabel(const QString &newText)
{
    statusLabel->setText(newText);
}

void DlgUpdate::warnInstallerIsWaiting()
{
    // Modeless on purpose: the installer asks the application to close exactly once - the NSIS
    // template closes matching windows itself (CloseMatchingApps in NSIS.template.in), Setup asks
    // through Restart Manager - and then gives up on the update, while Qt drops spontaneous close
    // events - a WM_CLOSE from the installer - for windows that are blocked by a modal widget
    // (QGuiApplicationPrivate::processCloseEvent). An application modal message box here would
    // therefore swallow the installer's one and only request.
    auto *notification = new QMessageBox(QMessageBox::Warning, tr("Update"),
                                         tr("The update installer is waiting for Cockatrice to close. "
                                            "Cockatrice is still busy, so save your work and close it. "
                                            "Otherwise the update cannot be installed and is cancelled."),
                                         QMessageBox::Ok, parentWidget());
    notification->setWindowModality(Qt::NonModal);
    notification->setAttribute(Qt::WA_DeleteOnClose);
    notification->show();
}

void DlgUpdate::updateCheckError(const QString &errorString)
{
    setLabel(tr("Error"));
    QMessageBox::critical(this, tr("Update Error"),
                          tr("An error occurred while checking for updates:") + QString(" ") + errorString);
}

void DlgUpdate::downloadError(const QString &errorString)
{
    setLabel(tr("Error"));
    enableUpdateButton(true);
    QMessageBox::critical(this, tr("Update Error"),
                          tr("An error occurred while downloading an update:") + QString(" ") + errorString);
}

void DlgUpdate::downloadSuccessful(const QUrl &filepath)
{
    setLabel(tr("Installing..."));

    QString installerPath = filepath.toLocalFile();

    const QString overridePath = QDir(QFileInfo(installerPath).absolutePath()).filePath(UPDATE_INSTALLER_OVERRIDE);
    if (QFileInfo::exists(overridePath)) {
        qCInfo(DlgUpdateLog) << "Installing the update installer override instead of the download:" << overridePath;
        installerPath = overridePath;
    }

    QString appDir = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    QProcess process;
    process.setProgram(installerPath);

    // One command line has to drive either installer, because a package can be built with either
    // one and the client cannot tell which it downloaded:
    //  - /R runs the NSIS installer silently and reinstalls over the existing installation. Setup
    //    ignores the switches it does not know, and that covers /R, /PID= and /D=.
    //  - /VERYSILENT and the switches around it are Setup's spelling of the same thing.
    //    /CLOSEAPPLICATIONS lets Restart Manager close Cockatrice if this process is still shutting
    //    down once Setup reaches the files, and /NOFORCECLOSEAPPLICATIONS keeps that from ever
    //    forcing it: a window that refuses to close still cancels the update rather than losing the
    //    user's work.
    //  - /PID= tells the NSIS installer which process to wait for before it replaces anything. We
    //    launch the installer first and only quit afterwards, so it starts while this process is
    //    still alive and still has every Qt runtime DLL mapped, and a mapped DLL cannot be deleted.
    //    Without the handle the installer can only poll for an image name and guess, and a wrong
    //    guess is what leaves the update writing new executables next to runtime files of the
    //    previous version, which fails on the next start with "The procedure entry point ... could
    //    not be located in the dynamic link library ...Qt6QuickWidgets.dll". Setup ignores the
    //    switch; Restart Manager releases the same handles, which is what /CLOSEAPPLICATIONS above
    //    asks it to do.
    //  - /UPDATER marks an install that the in-app updater launched. Both installers ignore it
    //    while running, and only the Inno Setup package reads it: it is what lets the failure
    //    hook offer the log of a silent update that did not finish, without ever putting a
    //    message box in front of a package manager's silent install.
    // NSIS honours /D= only as the last parameter and requires it unquoted even when it has spaces,
    // so it stays last here and the whole line goes through QProcess::setNativeArguments.
#ifdef Q_OS_WIN
    process.setNativeArguments(QString("/R /PID=%1 /UPDATER /VERYSILENT /SUPPRESSMSGBOXES /NORESTART"
                                       " /CLOSEAPPLICATIONS /NOFORCECLOSEAPPLICATIONS /D=%2")
                                   .arg(QCoreApplication::applicationPid())
                                   .arg(appDir));
#else
    // Linux/macOS: normal argument passing (not relevant since they update differently.)
    process.setArguments({"/R", QString("/D=%1").arg(appDir)});
#endif

    // Try to open the installer. If it opens, quit Cockatrice
    if (process.startDetached()) {
        qCInfo(DlgUpdateLog) << "Opened update installer successfully - closing Cockatrice";
        // Close the main window synchronously so file locks are released before the installer
        // (already launched) starts replacing files. This also flushes settings and shuts down the
        // tabs, but only when the close is actually accepted: MainWindow may veto it for a running
        // card DB update, an open game, or an unsaved deck, and closeForUpdate() also reports a
        // close already in progress (reached from a nested event loop while a shutdown prompt is
        // up). Only quit when the shutdown really ran - otherwise keep running so the user can
        // resolve the blocker, and warn them that the installer cannot finish until it closes.
        if (auto *window = qobject_cast<MainWindow *>(parent())) {
            if (window->closeForUpdate()) {
                QTimer::singleShot(0, qApp, [] { QCoreApplication::exit(0); });
            } else {
                warnInstallerIsWaiting();
            }
        } else {
            // Not a MainWindow, so no faithful close can be requested - but the parent still gets
            // its close() call, which is what the code before this change did and lets it flush
            // settings and shut down its tabs. Leaving is then unconditional: the installer is
            // already running against a process that still holds locks on the files it replaces.
            if (auto *widget = parentWidget()) {
                widget->close();
            }
            QTimer::singleShot(0, qApp, [] { QCoreApplication::exit(0); });
        }
        close();
    } else {
        setLabel(tr("Error"));
        QMessageBox::critical(this, tr("Update Error"),
                              tr("Cockatrice is unable to open the installer.") + "<br><br>" +
                                  tr("Try to update manually by closing Cockatrice and running the installer.") +
                                  "<br>" + tr("Download location") + QString(": %1").arg(filepath.toLocalFile()));
    }
}

void DlgUpdate::downloadProgressMade(qint64 bytesRead, qint64 totalBytes)
{
    progress->setMaximum(totalBytes);
    progress->setValue(bytesRead);
}
