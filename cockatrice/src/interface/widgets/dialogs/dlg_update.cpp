#include "dlg_update.h"

#include "../../../client/network/update/client/release_notes.h"
#include "../../../client/settings/cache_settings.h"
#include "../../theme_icon.h"
#include "../client/network/update/client/client_update_checker.h"
#include "../client/network/update/client/release_channel.h"
#include "../interface/window_main.h"
#include "update/client/update_downloader.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
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
#include <QTextBrowser>
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

QString DlgUpdate::localizedChannelName() const
{
    return tr(SettingsCache::instance().getUpdateReleaseChannel()->getName().toUtf8());
}

DlgUpdate::DlgUpdate(QWidget *parent, Release *knownRelease, bool knownCompatible) : QDialog(parent)
{
    setObjectName("updateDialog");
    setWindowTitle(tr("Cockatrice Update"));
    setMinimumSize(460, 340);
    resize(560, 520);

    headerIcon = new QLabel(this);
    headerIcon->setObjectName("updateHeaderIcon");
    headerIcon->setPixmap(loadThemedIcon(QStringLiteral("icons/arrow_to_bottom"), IconTint::primary())
                              .pixmap(QSize(32, 32), qApp->devicePixelRatio()));
    headerIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // A download glyph belongs to an offered update only: "checking" and "up to date" have
    // nothing to download, and the arrow would promise otherwise.
    headerIcon->setVisible(false);

    titleLabel = new QLabel(this);
    titleLabel->setObjectName("updateTitleLabel");
    QFont titleFont = QtUtils::boldFont(titleLabel->font());
    if (titleFont.pointSizeF() > 0) {
        titleFont.setPointSizeF(titleFont.pointSizeF() * 1.3);
    }
    titleLabel->setFont(titleFont);
    titleLabel->setWordWrap(true);

    versionLabel = new QLabel(this);
    versionLabel->setObjectName("updateVersionLabel");
    versionLabel->setWordWrap(true);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("updateStatusLabel");
    statusLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    statusLabel->setWordWrap(true);

    notesView = new QTextBrowser(this);
    notesView->setObjectName("updateNotesView");
    notesView->setOpenExternalLinks(true);
    notesView->setMinimumHeight(180);
    notesView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    progress = new QProgressBar(this);
    progress->setObjectName("updateProgress");
    progress->setVisible(false);

    actionButton = new QPushButton(this);
    actionButton->setObjectName("updateDownloadButton");
    closeButton = new QPushButton(tr("Close"), this);
    closeButton->setObjectName("updateCloseButton");
    stopDownload = new QPushButton(tr("Cancel Download"), this);
    stopDownload->setObjectName("updateCancelDownloadButton");

    buttonBox = new QDialogButtonBox(this);
    // ActionRole, not AcceptRole: a download starts inside the dialog and must not close it.
    buttonBox->addButton(actionButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(stopDownload, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);
    // QDialogButtonBox shows a button when it is added, so hide the two action buttons after
    // the box exists; showRelease() and setDownloading() decide when each one returns.
    actionButton->setVisible(false);
    stopDownload->setVisible(false);

    connect(actionButton, &QPushButton::clicked, this, &DlgUpdate::startDownload);
    connect(stopDownload, &QPushButton::clicked, this, &DlgUpdate::cancelDownload);
    connect(closeButton, &QPushButton::clicked, this, &DlgUpdate::reject);

    auto *headerText = QtUtils::makeLayout<QVBoxLayout>(nullptr, 4);
    headerText->addWidget(titleLabel);
    headerText->addWidget(versionLabel);

    auto *header = QtUtils::makeLayout<QHBoxLayout>(nullptr, 12);
    header->addWidget(headerIcon, 0, Qt::AlignTop);
    header->addLayout(headerText, 1);

    auto *layout = QtUtils::makeLayout<QVBoxLayout>(this, 12, 12);
    layout->addLayout(header);
    layout->addWidget(notesView, 1);
    layout->addWidget(statusLabel);
    layout->addWidget(progress);
    layout->addWidget(buttonBox);
    setLayout(layout);

    // Initialize the checker and downloader class
    uDownloader = new UpdateDownloader(this);
    connect(uDownloader, &UpdateDownloader::downloadSuccessful, this, &DlgUpdate::downloadSuccessful);
    connect(uDownloader, &UpdateDownloader::progressMade, this, &DlgUpdate::downloadProgressMade);
    connect(uDownloader, &UpdateDownloader::error, this, &DlgUpdate::downloadError);

    if (knownRelease != nullptr) {
        // The startup check already fetched everything below; asking again would only repeat
        // the request and make the dialog wait for it.
        showRelease(knownRelease, true, knownCompatible);
    } else {
        beginUpdateCheck();
    }
}

void DlgUpdate::done(int result)
{
    // The download belongs to this dialog: leaving it any way (Close, Escape, the window
    // button) also gives up the partial file.
    if (downloading) {
        cancelDownload();
    }
    QDialog::done(result);
}

void DlgUpdate::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    markNotesShown();
}

void DlgUpdate::markNotesShown()
{
    if (!isVisible() || release == nullptr || !updateOffered || shownNotesRelease == release->getName()) {
        return;
    }

    shownNotesRelease = release->getName();
    emit notesShown(release);
}

void DlgUpdate::gotoDownloadPage()
{
    QDesktopServices::openUrl(SettingsCache::instance().getUpdateReleaseChannel()->getManualDownloadUrl());
}

void DlgUpdate::beginUpdateCheck()
{
    titleLabel->setText(tr("Checking for updates"));
    versionLabel->setText(tr("Current version: %1").arg(VERSION_STRING));
    notesView->setVisible(false);

    progress->setMinimum(0);
    progress->setMaximum(0);
    progress->setVisible(true);

    auto checker = new ClientUpdateChecker(this);
    connect(checker, &ClientUpdateChecker::finishedCheck, this, &DlgUpdate::finishedUpdateCheck);
    connect(checker, &ClientUpdateChecker::error, this, &DlgUpdate::updateCheckError);
    checker->check();
}

void DlgUpdate::finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *newRelease)
{
    emit checkFinished(needToUpdate, isCompatible, newRelease);

    if (newRelease == nullptr) {
        progress->setVisible(false);
        setStatus(tr("Finished checking for updates."));
        return;
    }

    showRelease(newRelease, needToUpdate, isCompatible);
}

void DlgUpdate::showRelease(Release *newRelease, bool needToUpdate, bool isCompatible)
{
    release = newRelease;
    updateOffered = needToUpdate;
    headerIcon->setVisible(needToUpdate);
    // Without SSL a download would fail after the user already agreed to it; offer the
    // download page instead, exactly like an incompatible platform.
    compatible = isCompatible && QSslSocket::supportsSsl();

    progress->setVisible(false);
    setDownloading(false);
    setStatus({});

    if (!needToUpdate) {
        titleLabel->setText(tr("Cockatrice is up to date"));
        versionLabel->setText(tr("You are running the latest version in the %1 channel (version %2).")
                                  .arg(localizedChannelName(), VERSION_STRING));
        notesView->setVisible(false);
        actionButton->setVisible(false);
        closeButton->setDefault(true);
        return;
    }

    titleLabel->setText(tr("Update Available"));
    versionLabel->setText(tr("%1 (released %2, %3 channel)")
                              .arg(release->getName(),
                                   release->getPublishDate().toString(QLocale().dateFormat(QLocale::LongFormat)),
                                   localizedChannelName()));

    const QString curated = curatedReleaseNotes(release->getChangelog());
    // An uncurated release still gets the version details and the release page; it never falls
    // back to the generated commit list the body also carries.
    const QString notes =
        (curated.isEmpty() ? tr("There are no release notes for this version.") : curated) +
        QStringLiteral("\n\n[%1](%2)").arg(tr("Read the full release notes on GitHub"), release->getDescriptionUrl());
    notesView->setMarkdown(notes);
    notesView->setVisible(true);

    // Clearing the unread mark belongs to the moment the notes are on screen, not to the
    // moment they are rendered: a dialog built from the startup result only becomes visible
    // on exec(), and showEvent() reports it then.
    markNotesShown();

    actionButton->setVisible(true);
    if (compatible) {
        actionButton->setText(tr("Download Update"));
        actionButton->setDefault(true);
        closeButton->setDefault(false);
    } else {
        actionButton->setText(tr("Open Download Page"));
        actionButton->setDefault(true);
        closeButton->setDefault(false);
        setStatus(QSslSocket::supportsSsl()
                      ? tr("Automatic updates are not available for this system. Download the new version from the "
                           "releases page instead.")
                      : tr("This build has no SSL support, so updates cannot be downloaded automatically. Download "
                           "the new version from the releases page instead."));
    }
}

void DlgUpdate::startDownload()
{
    if (!compatible || release == nullptr) {
        gotoDownloadPage();
        return;
    }

    setDownloading(true);
    progress->setMinimum(0);
    progress->setMaximum(0);
    setStatus(tr("Downloading update: %1").arg(release->getName()));
    uDownloader->beginDownload(release->getDownloadUrl());
}

void DlgUpdate::cancelDownload()
{
    if (!downloading) {
        return;
    }

    emit uDownloader->stopDownload();
    setDownloading(false);
    progress->setVisible(false);
    setStatus(tr("Download canceled."));
    if (compatible) {
        actionButton->setText(tr("Download Update"));
        actionButton->setVisible(true);
        actionButton->setDefault(true);
    }
}

void DlgUpdate::setDownloading(bool value)
{
    downloading = value;
    stopDownload->setVisible(value);
    progress->setVisible(value);
    if (value) {
        actionButton->setVisible(false);
        actionButton->setDefault(false);
        closeButton->setDefault(false);
    }
}

void DlgUpdate::setStatus(const QString &text)
{
    statusLabel->setText(text);
    statusLabel->setVisible(!text.isEmpty());
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
    progress->setVisible(false);
    // A failed check has its own verdict; leaving the "checking" headline up contradicts the
    // status line right below it, and the dialog then has no primary action at all.
    titleLabel->setText(tr("Could not check for updates"));
    notesView->setVisible(false);
    actionButton->setVisible(false);
    closeButton->setDefault(true);
    setStatus(errorString);
}

void DlgUpdate::downloadError(const QString &errorString)
{
    setDownloading(false);
    progress->setVisible(false);
    setStatus(tr("Could not download the update: %1").arg(errorString));
    if (compatible) {
        actionButton->setText(tr("Download Update"));
        actionButton->setVisible(true);
        actionButton->setDefault(true);
    }
}

void DlgUpdate::downloadSuccessful(const QUrl &filepath)
{
    setDownloading(false);
    progress->setVisible(false);
    setStatus(tr("Installing..."));

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
    // Linux/macOS: normal argument passing (not relevant since they update differently.)
    process.setArguments({"/R", QString("/D=%1").arg(appDir)});
#endif

    // Try to open the installer. If it opens, quit Cockatrice
    if (process.startDetached()) {
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
        setStatus(tr("Cockatrice is unable to open the installer. Try to update manually by closing Cockatrice and "
                     "running the installer. Download location: %1")
                      .arg(filepath.toLocalFile()));
    }
}

void DlgUpdate::downloadProgressMade(qint64 bytesRead, qint64 totalBytes)
{
    if (totalBytes <= 0) {
        // Qt reports -1 when the server sends no content length; stay indeterminate.
        progress->setMaximum(0);
        return;
    }
    progress->setMaximum(static_cast<int>(totalBytes));
    progress->setValue(static_cast<int>(bytesRead));
}
