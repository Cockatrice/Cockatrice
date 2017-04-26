#include <QtNetwork>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QVBoxLayout>

#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <version_string.h>

#include "dlg_update.h"
#include "releasechannel.h"
#include "settingscache.h"
#include "window_main.h"

DlgUpdate::DlgUpdate(QWidget *parent) : QDialog(parent) {

    //Handle layout
    statusLabel = new QLabel(this);
    statusLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    statusLabel->setWordWrap(true);
    descriptionLabel = new QLabel(tr("Current release channel:") + " " + tr(settingsCache->getUpdateReleaseChannel()->getName().toUtf8()), this);
    progress = new QProgressBar(this);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setFixedWidth(350);

    ok = new QPushButton("Close", this);
    manualDownload = new QPushButton(tr("Reinstall"), this);
    stopDownload = new QPushButton(tr("Cancel Download"), this);
    gotoDownload = new QPushButton(tr("Open Download Page"), this);

    addStopDownloadAndRemoveOthers(false); // Add all buttons to box
    enableUpdateButton(false); //Unless we know there's an update available, you can't install
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

    //Check for SSL (this probably isn't necessary)
    if (!QSslSocket::supportsSsl()) {
        enableUpdateButton(false);
        QMessageBox::critical(this, tr("Error"),
            tr("Cockatrice was not built with SSL support, so you cannot download updates automatically! "
            "Please visit the download page to update manually."));
    }

    //Initialize the checker and downloader class
    uDownloader = new UpdateDownloader(this);
    connect(uDownloader, SIGNAL(downloadSuccessful(QUrl)), this, SLOT(downloadSuccessful(QUrl)));
    connect(uDownloader, SIGNAL(progressMade(qint64, qint64)), this, SLOT(downloadProgressMade(qint64, qint64)));
    connect(uDownloader, SIGNAL(error(QString)), this, SLOT(downloadError(QString)));

    ReleaseChannel * channel = settingsCache->getUpdateReleaseChannel();
    connect(channel, SIGNAL(finishedCheck(bool, bool, Release *)), this, SLOT(finishedUpdateCheck(bool, bool, Release *)));
    connect(channel, SIGNAL(error(QString)), this, SLOT(updateCheckError(QString)));

    //Check for updates
    beginUpdateCheck();
}

void DlgUpdate::closeDialog() {
    accept();
}


void DlgUpdate::gotoDownloadPage() {
    QDesktopServices::openUrl(settingsCache->getUpdateReleaseChannel()->getManualDownloadUrl());
}

void DlgUpdate::downloadUpdate() {
    setLabel(tr("Downloading update..."));
    addStopDownloadAndRemoveOthers(true); // Will remove all other buttons
    uDownloader->beginDownload(updateUrl);
}

void DlgUpdate::cancelDownload() {
    emit uDownloader->stopDownload();
    setLabel("Download Canceled");
    addStopDownloadAndRemoveOthers(false);
    downloadProgressMade(0, 1);
}

void DlgUpdate::beginUpdateCheck() {
    progress->setMinimum(0);
    progress->setMaximum(0);
    setLabel(tr("Checking for updates..."));
    settingsCache->getUpdateReleaseChannel()->checkForUpdates();
}

void DlgUpdate::finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *release) {

    QString publishDate, versionName;

    //Update the UI to say we've finished
    progress->setMaximum(100);
    setLabel(tr("Finished checking for updates."));

    //If there are no available builds, then they can't auto update.
    enableUpdateButton(isCompatible);

    //Give the user the appropriate message
    if (!needToUpdate) {
        //If there's no need to update, tell them that. However we still allow them to run the
        //downloader themselves if there's a compatible build
        QMessageBox::information(this, tr("Cockatrice Update"), tr("Your version of Cockatrice is up to date."));
        setLabel(tr("You are already running the latest %1 release - %2").arg(tr(settingsCache->getUpdateReleaseChannel()->getName().toUtf8())).arg(VERSION_STRING));
        return;
    }

    publishDate = release->getPublishDate().toString(Qt::DefaultLocaleLongDate);
    if (isCompatible) {
        //If there is an update, save its URL and work out its name
        updateUrl = release->getDownloadUrl();

        int reply;
        reply = QMessageBox::question(this, "Update Available",
            tr("A new version is available:<br/>%1<br/>published on %2 ."
            "<br/>More informations are available on the <a href=\"%3\">release changelog</a>"
            "<br/>Do you want to update now?").arg(release->getName(), publishDate, release->getDescriptionUrl()),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
            downloadUpdate();
    } else {
        QMessageBox::information(this, tr("Cockatrice Update"),
            tr("A new version is available:<br/>%1<br/>published on %2 ."
            "<br/>More informations are available on the <a href=\"%3\">release changelog</a>"
            "<br/>Unfortunately there are no packages available for your operating system. "
            "You may have to use a developer build or build from source yourself. Please visit the download page.").arg(release->getName(), publishDate, release->getDescriptionUrl()));
    }
}

void DlgUpdate::enableUpdateButton(bool enable) {
    manualDownload->setEnabled(enable);
}

void DlgUpdate::addStopDownloadAndRemoveOthers(bool enable) {
    if (enable) {
        buttonBox->addButton(stopDownload, QDialogButtonBox::ActionRole);
        buttonBox->removeButton(manualDownload);
        buttonBox->removeButton(gotoDownload);
    }
    else {
        buttonBox->removeButton(stopDownload);
        buttonBox->addButton(manualDownload, QDialogButtonBox::ActionRole);
        buttonBox->addButton(gotoDownload, QDialogButtonBox::ActionRole);
    }
}

void DlgUpdate::enableOkButton(bool enable) {
    ok->setEnabled(enable);
}

void DlgUpdate::setLabel(QString newText) {
    statusLabel->setText(newText);
}

void DlgUpdate::updateCheckError(QString errorString) {
    setLabel("Error");
    QMessageBox::critical(this, tr("Update Error"), tr("An error occurred while checking for updates: ") + errorString);
}

void DlgUpdate::downloadError(QString errorString) {
    setLabel(tr("Error"));
    enableUpdateButton(true);
    QMessageBox::critical(this, tr("Update Error"), tr("An error occurred while downloading an update: ") + errorString);
}

void DlgUpdate::downloadSuccessful(QUrl filepath) {
    setLabel(tr("Installing..."));
    //Try to open the installer. If it opens, quit Cockatrice
    if (QDesktopServices::openUrl(filepath))
    {
        QMetaObject::invokeMethod(static_cast<MainWindow *>(parent()), "close", Qt::QueuedConnection);
        qDebug() << "Opened downloaded update file successfully - closing Cockatrice";
        close();
    } else {
        setLabel(tr("Error"));
        QMessageBox::critical(this, tr("Update Error"),
            tr("Unable to open the installer. You might be able to manually update by closing Cockatrice and running the installer at %1.").arg(filepath.toLocalFile()));
    }
}

void DlgUpdate::downloadProgressMade(qint64 bytesRead, qint64 totalBytes) {
    progress->setMaximum(totalBytes);
    progress->setValue(bytesRead);
}
