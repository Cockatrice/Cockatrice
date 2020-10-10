#include "dlg_update.h"

#include "releasechannel.h"
#include "settingscache.h"
#include "window_main.h"

#include <QApplication>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtNetwork>
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

    connect(gotoDownload, SIGNAL(clicked()), this, SLOT(gotoDownloadPage()));
    connect(manualDownload, SIGNAL(clicked()), this, SLOT(downloadUpdate()));
    connect(stopDownload, SIGNAL(clicked()), this, SLOT(cancelDownload()));
    connect(ok, SIGNAL(clicked()), this, SLOT(closeDialog()));

    QVBoxLayout *parentLayout = new QVBoxLayout(this);
    parentLayout->addWidget(descriptionLabel);
    parentLayout->addWidget(statusLabel);
    parentLayout->addWidget(progress);
    parentLayout->addWidget(buttonBox);

    setLayout(parentLayout);

    // Check for SSL (this probably isn't necessary)
    if (!QSslSocket::supportsSsl()) {
        enableUpdateButton(false);
        QMessageBox::critical(this, tr("Error"),
                              tr("Cockatrice was not built with SSL support, therefore you cannot download updates "
                                 "automatically! \nPlease visit the download page to update manually."));
    }

    // Initialize the checker and downloader class
    uDownloader = new UpdateDownloader(this);
    connect(uDownloader, SIGNAL(downloadSuccessful(QUrl)), this, SLOT(downloadSuccessful(QUrl)));
    connect(uDownloader, SIGNAL(progressMade(qint64, qint64)), this, SLOT(downloadProgressMade(qint64, qint64)));
    connect(uDownloader, SIGNAL(error(QString)), this, SLOT(downloadError(QString)));

    ReleaseChannel *channel = SettingsCache::instance().getUpdateReleaseChannel();
    connect(channel, SIGNAL(finishedCheck(bool, bool, Release *)), this,
            SLOT(finishedUpdateCheck(bool, bool, Release *)));
    connect(channel, SIGNAL(error(QString)), this, SLOT(updateCheckError(QString)));

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

void DlgUpdate::downloadUpdate()
{
    setLabel(tr("Downloading update..."));
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
    SettingsCache::instance().getUpdateReleaseChannel()->checkForUpdates();
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

    publishDate = release->getPublishDate().toString(Qt::DefaultLocaleLongDate);
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
            downloadUpdate();
    } else {
        QMessageBox::information(
            this, tr("Update Available"),
            tr("A new version of Cockatrice is available!") + "<br><br>" + "<b>" + tr("New version") +
                QString(":</b> %1<br>").arg(release->getName()) + "<b>" + tr("Released") +
                QString(":</b> %1 (<a href=\"%2\">").arg(publishDate, release->getDescriptionUrl()) + tr("Changelog") +
                "</a>)<br><br>" +
                tr("Unfortunately there are no download packages available for your operating system. \nYou may have "
                   "to build from source yourself.") +
                "<br><br>" +
                tr("Please check the download page manually and visit the wiki for instructions on compiling."));
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

void DlgUpdate::setLabel(QString newText)
{
    statusLabel->setText(newText);
}

void DlgUpdate::updateCheckError(QString errorString)
{
    setLabel(tr("Error"));
    QMessageBox::critical(this, tr("Update Error"),
                          tr("An error occurred while checking for updates:") + QString(" ") + errorString);
}

void DlgUpdate::downloadError(QString errorString)
{
    setLabel(tr("Error"));
    enableUpdateButton(true);
    QMessageBox::critical(this, tr("Update Error"),
                          tr("An error occurred while downloading an update:") + QString(" ") + errorString);
}

void DlgUpdate::downloadSuccessful(QUrl filepath)
{
    setLabel(tr("Installing..."));
    // Try to open the installer. If it opens, quit Cockatrice
    if (QDesktopServices::openUrl(filepath)) {
        QMetaObject::invokeMethod(static_cast<MainWindow *>(parent()), "close", Qt::QueuedConnection);
        qDebug() << "Opened downloaded update file successfully - closing Cockatrice";
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
