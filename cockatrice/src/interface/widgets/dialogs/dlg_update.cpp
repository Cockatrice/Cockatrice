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
#include <QPointer>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QSslSocket>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <qnamespace.h>
#include <version_string.h>

namespace
{

int showUpdateMessage(QWidget *parent,
                      QMessageBox::Icon icon,
                      const QString &title,
                      const QString &text,
                      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
{
    // Static QMessageBox helpers create a stack dialog parented to the caller.
    // Parent destruction during exec() would delete that stack object. Keep the
    // prompt on the heap and observe its lifetime through the nested event loop.
    QPointer<QMessageBox> message = new QMessageBox(icon, title, text, buttons, parent);
    if (defaultButton != QMessageBox::NoButton) {
        message->setDefaultButton(defaultButton);
    }
    const int result = message->exec();
    delete message.data();
    return result;
}

} // namespace

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
    connect(manualDownload, &QPushButton::clicked, this, [this] { downloadUpdate(updateReleaseName); });
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
        showUpdateMessage(this, QMessageBox::Critical, tr("Error"),
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
    const QPointer<DlgUpdate> dialog(this);
    if (downloading) {
        cancelDownload();
    }
    if (dialog) {
        dialog->accept();
    }
}

void DlgUpdate::gotoDownloadPage()
{
    QDesktopServices::openUrl(SettingsCache::instance().getUpdateReleaseChannel()->getManualDownloadUrl());
}

void DlgUpdate::downloadUpdate(const QString &releaseName)
{
    if (downloading || updateUrl.isEmpty() || !updateUrl.isValid()) {
        return;
    }
    downloading = true;
    setLabel(tr("Downloading update: %1").arg(releaseName));
    progress->setRange(0, 0);
    addStopDownloadAndRemoveOthers(true);
    uDownloader->beginDownload(updateUrl);
}

void DlgUpdate::cancelDownload()
{
    if (!downloading) {
        return;
    }
    // Abort can synchronously emit reply signals. Mark the transfer inactive
    // first, so they cannot turn cancellation into another error dialog.
    downloading = false;
    setLabel(tr("Download canceled"));
    addStopDownloadAndRemoveOthers(false);
    manualDownload->setText(tr("Retry download"));
    enableUpdateButton(!updateUrl.isEmpty());
    progress->setRange(0, 1000);
    progress->setValue(0);
    emit uDownloader->stopDownload();
}

void DlgUpdate::beginUpdateCheck()
{
    updateUrl.clear();
    updateReleaseName.clear();
    enableUpdateButton(false);
    progress->setRange(0, 0);
    setLabel(tr("Checking for updates..."));

    auto checker = new ClientUpdateChecker(this);
    connect(checker, &ClientUpdateChecker::finishedCheck, this, &DlgUpdate::finishedUpdateCheck);
    connect(checker, &ClientUpdateChecker::error, this, &DlgUpdate::updateCheckError);
    connect(checker, &ClientUpdateChecker::finishedCheck, checker, &QObject::deleteLater);
    connect(checker, &ClientUpdateChecker::error, checker, &QObject::deleteLater);
    checker->check();
}

void DlgUpdate::finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *release)
{
    if (!release) {
        updateCheckError(tr("The update server did not provide a release."));
        return;
    }
    updateReleaseName = release->getName();
    const QString channelName = SettingsCache::instance().getUpdateReleaseChannel()->getName().toHtmlEscaped();
    const QString releaseName = updateReleaseName.toHtmlEscaped();
    const QString descriptionUrl = release->getDescriptionUrl().toHtmlEscaped();
    const QString selectedChannel = "<b>" + tr("Selected release channel") + QString(":</b> %1").arg(channelName);

    // Update the UI to say we've finished
    progress->setRange(0, 1000);
    progress->setValue(1000);
    setLabel(tr("Finished checking for updates"));

    // If there are no available builds, then they can't auto update.
    updateUrl = isCompatible ? QUrl(release->getDownloadUrl()) : QUrl();
    isCompatible = isCompatible && updateUrl.isValid() && !updateUrl.isEmpty();
    enableUpdateButton(isCompatible);
    manualDownload->setText(needToUpdate ? tr("Download update") : tr("Reinstall"));

    // Give the user the appropriate message
    if (!needToUpdate) {
        // Closing this result still closes the check, but a compatible release
        // can also be reinstalled without having to leave for the website.
        const QPointer<DlgUpdate> dialog(this);
        QPointer<QMessageBox> notice = new QMessageBox(
            QMessageBox::Information, tr("No Update Available"),
            tr("Cockatrice is up to date!") + "<br><br>" +
                tr("You are already running the latest version available in the chosen release channel.") + "<br>" +
                "<b>" + tr("Current version") +
                QString(":</b> <a href=\"%1\">%2</a><br>").arg(descriptionUrl, releaseName) + selectedChannel,
            QMessageBox::Close, this);
        QPushButton *reinstall = isCompatible ? notice->addButton(tr("Reinstall"), QMessageBox::AcceptRole) : nullptr;
        notice->setDefaultButton(QMessageBox::Close);
        notice->exec();
        if (!notice) {
            return;
        }
        const bool reinstallRequested = reinstall && notice->clickedButton() == reinstall;
        delete notice.data();
        if (!dialog) {
            return;
        }
        if (reinstallRequested) {
            dialog->downloadUpdate(dialog->updateReleaseName);
        } else {
            dialog->closeDialog();
        }
        return;
    }

    const QString publishDate = release->getPublishDate().toString(QLocale().dateFormat(QLocale::LongFormat));
    if (isCompatible) {
        const QString message = tr("A new version of Cockatrice is available!") + "<br><br>" + "<b>" +
                                tr("New version") + QString(":</b> %1<br>").arg(releaseName) + "<b>" + tr("Released") +
                                QString(":</b> %1 (<a href=\"%2\">").arg(publishDate, descriptionUrl) +
                                tr("Changelog") + "</a>)<br>" + selectedChannel + "<br><br>" +
                                tr("Do you want to update now?");

        const QPointer<DlgUpdate> dialog(this);
        const bool confirmed =
            showUpdateMessage(this, QMessageBox::Question, tr("Update Available"), message,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
        if (!dialog) {
            return;
        }
        if (confirmed) {
            dialog->downloadUpdate(dialog->updateReleaseName);
        } else {
            dialog->closeDialog();
        }
    } else {
        showUpdateMessage(
            this, QMessageBox::Information, tr("Update Available"),
            tr("A new version of Cockatrice is available!") + "<br><br>" + "<b>" + tr("New version") +
                QString(":</b> %1<br>").arg(releaseName) + "<b>" + tr("Released") +
                QString(":</b> %1 (<a href=\"%2\">").arg(publishDate, descriptionUrl) + tr("Changelog") + "</a>)<br>" +
                selectedChannel + "<br><br>" +
                tr("Unfortunately, the automatic updater failed to find a compatible download. \nYou may have to "
                   "manually download the new version.") +
                "<br><br>" +
                tr("Please check the <a href=\"%1\">releases page</a> on our Github and download the build for your "
                   "system.")
                    .arg(descriptionUrl));
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
    stopDownload->setVisible(enable);
    manualDownload->setVisible(!enable);
    gotoDownload->setVisible(!enable);
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
    // Modeless on purpose: the installer asks the application to close exactly once (see
    // CloseMatchingApps in NSIS.template.in) and only polls afterwards, while Qt drops spontaneous
    // close events - a WM_CLOSE from the installer - for windows that are blocked by a modal
    // widget (QGuiApplicationPrivate::processCloseEvent). An application modal message box here
    // would therefore swallow the installer's one and only request for the rest of its wait.
    auto *notification = new QMessageBox(QMessageBox::Warning, tr("Update"),
                                         tr("The update installer is already running and waits about a "
                                            "minute for Cockatrice to close. Cockatrice is still busy, so "
                                            "save your work and close it before then. Otherwise the "
                                            "installer gives up and the update is cancelled."),
                                         QMessageBox::Ok, parentWidget());
    notification->setWindowModality(Qt::NonModal);
    notification->setAttribute(Qt::WA_DeleteOnClose);
    notification->show();
}

void DlgUpdate::updateCheckError(const QString &errorString)
{
    progress->setRange(0, 1000);
    progress->setValue(0);
    enableUpdateButton(false);
    setLabel(tr("Error"));
    showUpdateMessage(this, QMessageBox::Critical, tr("Update Error"),
                      tr("An error occurred while checking for updates:") + QString(" ") + errorString);
}

void DlgUpdate::downloadError(const QString &errorString)
{
    if (!downloading) {
        return;
    }
    downloading = false;
    addStopDownloadAndRemoveOthers(false);
    manualDownload->setText(tr("Retry download"));
    enableUpdateButton(!updateUrl.isEmpty());
    progress->setRange(0, 1000);
    progress->setValue(0);
    setLabel(tr("Error"));
    showUpdateMessage(this, QMessageBox::Critical, tr("Update Error"),
                      tr("An error occurred while downloading an update:") + QString(" ") + errorString);
}

void DlgUpdate::downloadSuccessful(const QUrl &filepath)
{
    if (!downloading) {
        return;
    }
    downloading = false;
    addStopDownloadAndRemoveOthers(false);
    enableUpdateButton(false);
    progress->setRange(0, 1000);
    progress->setValue(1000);

#if defined(Q_OS_MACOS)
    // macOS releases are disk images, not executable NSIS installers. A true
    // result means the OS accepted the open request, not that mounting or
    // installation has completed; the current session must remain available.
    const QPointer<DlgUpdate> dialog(this);
    setLabel(tr("Opening downloaded update..."));
    const bool openRequested = QDesktopServices::openUrl(filepath);
    if (!dialog) {
        return;
    }
    const QString downloadedPath = QDir::toNativeSeparators(filepath.toLocalFile()).toHtmlEscaped();
    if (openRequested) {
        setLabel(tr("Update downloaded"));
        showUpdateMessage(
            this, QMessageBox::Information, tr("Install Update"),
            tr("Cockatrice asked macOS to open the downloaded disk image.") + "<br><br>" +
                tr("After it opens, save your work and close Cockatrice before dragging the new applications "
                   "into the Applications folder.") +
                "<br><br>" + tr("If the disk image does not open, open this file in Finder:") + "<br>" +
                downloadedPath);
        if (dialog) {
            dialog->closeDialog();
        }
    } else {
        manualDownload->setText(tr("Retry download"));
        enableUpdateButton(!updateUrl.isEmpty());
        setLabel(tr("Error"));
        showUpdateMessage(
            this, QMessageBox::Critical, tr("Update Error"),
            tr("macOS could not open the downloaded disk image. Open it in Finder to install the update.") +
                "<br><br>" + tr("Download location") + ": " + downloadedPath);
    }
    return;
#endif

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

    // NSIS needs the /D= argument to be an UNQUOTED string, even if it contains spaces. Qt likes to quote arguments if
    // they contain spaces, so we use the windows exclusive QProcess::setNativeArguments in the only case where this is
    // relevant, which preserves the argument unquoted.
    //
    // /PID= tells the installer which process to wait for before it replaces anything. We launch the installer first
    // and only quit afterwards, so it starts while this process is still alive and still has every Qt runtime DLL
    // mapped, and a mapped DLL cannot be deleted. Without the handle the installer can only poll for an image name and
    // guess, and a wrong guess is what leaves the update writing new executables next to runtime files of the previous
    // version, which fails on the next start with "The procedure entry point ... could not be located in the dynamic
    // link library
    // ...Qt6QuickWidgets.dll". /D= has to stay the last parameter, which is why it is last here too.
#ifdef Q_OS_WIN
    process.setNativeArguments(QString("/R /PID=%1 /D=%2").arg(QCoreApplication::applicationPid()).arg(appDir));
#else
    // Normal argument passing for other platforms; macOS disk images are handled above.
    process.setArguments({"/R", QString("/D=%1").arg(appDir)});
#endif

    // Try to open the installer. If it opens, quit Cockatrice
    if (process.startDetached()) {
        const QPointer<DlgUpdate> dialog(this);
        qCInfo(DlgUpdateLog) << "Opened update installer successfully - closing Cockatrice";
        // Close the main window synchronously so file locks are released before the NSIS installer
        // (already launched) starts replacing files. This also flushes settings and shuts down the
        // tabs, but only when the close is actually accepted: MainWindow may veto it for a running
        // card DB update, an open game, or an unsaved deck, and closeForUpdate() also reports a
        // close already in progress (reached from a nested event loop while a shutdown prompt is
        // up). Only quit when the shutdown really ran - otherwise keep running so the user can
        // resolve the blocker, and warn them that the installer only waits about a minute
        // before it gives up and cancels the update.
        if (auto *window = qobject_cast<MainWindow *>(parent())) {
            if (window->closeForUpdate()) {
                QTimer::singleShot(0, qApp, [] { QCoreApplication::exit(0); });
            } else if (dialog) {
                dialog->warnInstallerIsWaiting();
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
        if (dialog) {
            dialog->close();
        }
    } else {
        manualDownload->setText(tr("Retry download"));
        enableUpdateButton(!updateUrl.isEmpty());
        setLabel(tr("Error"));
        showUpdateMessage(this, QMessageBox::Critical, tr("Update Error"),
                          tr("Cockatrice is unable to open the installer.") + "<br><br>" +
                              tr("Try to update manually by closing Cockatrice and running the installer.") + "<br>" +
                              tr("Download location") + QString(": %1").arg(filepath.toLocalFile()));
    }
}

void DlgUpdate::downloadProgressMade(qint64 bytesRead, qint64 totalBytes)
{
    if (!downloading) {
        return;
    }
    if (totalBytes <= 0) {
        progress->setRange(0, 0);
        return;
    }
    // QProgressBar uses int values; installer sizes and network counters use
    // qint64. Normalize without overflowing either type for multi-GB downloads.
    constexpr int scale = 1000;
    const qint64 received = qBound<qint64>(0, bytesRead, totalBytes);
    const int value = static_cast<int>((static_cast<long double>(received) / totalBytes) * scale);
    progress->setRange(0, scale);
    progress->setValue(value);
}
