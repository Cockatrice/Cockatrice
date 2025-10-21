#include "dlg_update.h"

#include "../client/network/update/client/client_update_checker.h"
#include "../client/network/update/client/release_channel.h"
#include "../interface/window_main.h"

#include <QApplication>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtNetwork>
#include <libcockatrice/settings/cache_settings.h>
#include <version_string.h>

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
    // TODO: make reinstall button actually do something when clicked
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
            this, tr("No Update Available"),
            tr("Cockatrice is up to date!") + "<br><br>" +
                tr("You are already running the latest version available in the chosen release channel.") + "<br>" +
                "<b>" + tr("Current version") + QString(":</b> %1<br>").arg(VERSION_STRING) + "<b>" +
                tr("Selected release channel") +
                QString(":</b> %1").arg(tr(SettingsCache::instance().getUpdateReleaseChannel()->getName().toUtf8())));
        return;
    }

    publishDate = release->getPublishDate().toString(QLocale().dateFormat(QLocale::LongFormat));
    if (isCompatible) {
        int reply;
        reply = QMessageBox::question(
            this, tr("Update Available"),
            tr("A new version of Cockatrice is available!") + "<br><br>" + "<b>" + tr("New version") +
                QString(":</b> %1<br>").arg(release->getName()) + "<b>" + tr("Released") +
                QString(":</b> %1 (<a href=\"%2\">").arg(publishDate, release->getDescriptionUrl()) + tr("Changelog") +
                "</a>)<br><br>" + tr("Do you want to update now?"),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
            downloadUpdate(release->getName());
    } else {
        QMessageBox::information(
            this, tr("Update Available"),
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
    // Try to open the installer. If it opens, quit Cockatrice
    if (QDesktopServices::openUrl(filepath)) {
        QMetaObject::invokeMethod(static_cast<MainWindow *>(parent()), "close", Qt::QueuedConnection);
        qCInfo(DlgUpdateLog) << "Opened downloaded update file successfully - closing Cockatrice";
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
