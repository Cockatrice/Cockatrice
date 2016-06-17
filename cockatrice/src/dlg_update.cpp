#define HUMAN_DOWNLOAD_URL "https://bintray.com/cockatrice/Cockatrice/Cockatrice/_latestVersion"
#define API_DOWNLOAD_BASE_URL "https://dl.bintray.com/cockatrice/Cockatrice/"
#define DATE_LENGTH 10
#define MAX_DATE_LENGTH 100
#define SHORT_SHA1_HASH_LENGTH 7

#include <QtNetwork>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>

#include "dlg_update.h"
#include "window_main.h"

DlgUpdate::DlgUpdate(QWidget *parent) : QDialog(parent) {

    //Handle layout
    text = new QLabel(this);
    progress = new QProgressBar(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    ok = new QPushButton("Ok", this);
    manualDownload = new QPushButton(tr("Update Anyway"), this);
    enableUpdateButton(false); //Unless we know there's an update available, you can't install
    gotoDownload = new QPushButton(tr("Open Download Page"), this);
    buttonBox->addButton(manualDownload, QDialogButtonBox::ActionRole);
    buttonBox->addButton(gotoDownload, QDialogButtonBox::ActionRole);
    buttonBox->addButton(ok, QDialogButtonBox::AcceptRole);

    connect(gotoDownload, SIGNAL(clicked()), this, SLOT(gotoDownloadPage()));
    connect(manualDownload, SIGNAL(clicked()), this, SLOT(downloadUpdate()));
    connect(ok, SIGNAL(clicked()), this, SLOT(closeDialog()));

    QVBoxLayout *parentLayout = new QVBoxLayout(this);
    parentLayout->addWidget(text);
    parentLayout->addWidget(progress);
    parentLayout->addWidget(buttonBox);

    setLayout(parentLayout);

    //Check for SSL (this probably isn't necessary)
    if (!QSslSocket::supportsSsl()) {
        enableUpdateButton(false);
        QMessageBox::critical(
                this,
                tr("Error"),
                tr("Cockatrice was not built with SSL support, so cannot download updates! "
                           "Please visit the download page and update manually."));
    }

    //Initialize the checker and downloader class
    uDownloader = new UpdateDownloader(this);
    connect(uDownloader, SIGNAL(downloadSuccessful(QUrl)), this, SLOT(downloadSuccessful(QUrl)));
    connect(uDownloader, SIGNAL(progressMade(qint64, qint64)),
            this, SLOT(downloadProgressMade(qint64, qint64)));
    connect(uDownloader, SIGNAL(error(QString)),
            this, SLOT(downloadError(QString)));

    uChecker = new UpdateChecker(this);
    connect(uChecker, SIGNAL(finishedCheck(bool, bool, QVariantMap * )),
            this, SLOT(finishedUpdateCheck(bool, bool, QVariantMap * )));
    connect(uChecker, SIGNAL(error(QString)),
            this, SLOT(updateCheckError(QString)));

    //Check for updates
    beginUpdateCheck();
}


void DlgUpdate::closeDialog() {
    accept();
}


void DlgUpdate::gotoDownloadPage() {
    QUrl openUrl(HUMAN_DOWNLOAD_URL);
    QDesktopServices::openUrl(openUrl);
}

void DlgUpdate::downloadUpdate() {
    setLabel(tr("Downloading update..."));
    enableOkButton(false);
    enableUpdateButton(false);
    uDownloader->beginDownload(updateUrl);
}

void DlgUpdate::beginUpdateCheck() {
    progress->setMinimum(0);
    progress->setMaximum(0);
    setLabel(tr("Checking for updates..."));
    uChecker->check();
}

void DlgUpdate::finishedUpdateCheck(bool needToUpdate, bool isCompatible, QVariantMap *build) {

    QString commitHash, commitDate;

    //Update the UI to say we've finished
    progress->setMaximum(100);
    setLabel(tr("Finished checking for updates."));

    //If there are no available builds, then they can't auto update.
    enableUpdateButton(isCompatible);

    //If there is an update, save its URL and work out its name
    if (isCompatible) {
        QString endUrl = (*build)["path"].toString();
        updateUrl = API_DOWNLOAD_BASE_URL + endUrl;
        commitHash = (*build)["sha1"].toString().left(SHORT_SHA1_HASH_LENGTH);
        commitDate = (*build)["created"].toString().remove(DATE_LENGTH, MAX_DATE_LENGTH);
    }

    //Give the user the appropriate message
    if (needToUpdate) {
        if (isCompatible) {

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Update Available",
                                          "A new build (commit " + commitHash + ") from " + commitDate +
                                          " is available. Download?",
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
                downloadUpdate();
        }
        else
        {
            QMessageBox::information(this, tr("Cockatrice Update"),
                                     tr("Your version of Cockatrice is out of date, but there are no packages"
                                                " available for your operating system. You may have to use a developer build or build from source"
                                                " yourself. Please visit the download page."));
        }
    }
    else {
        //If there's no need to update, tell them that. However we still allow them to run the
        //downloader themselves if there's a compatible build
        QMessageBox::information(this, tr("Cockatrice Update"), tr("Your version of Cockatrice is up to date."));
    }

}

void DlgUpdate::enableUpdateButton(bool enable) {
    manualDownload->setEnabled(enable);
}

void DlgUpdate::enableOkButton(bool enable) {
    ok->setEnabled(enable);
}

void DlgUpdate::setLabel(QString newText) {
    text->setText(newText);
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
        QMetaObject::invokeMethod((MainWindow*) parent(), "close", Qt::QueuedConnection);
        close();
    } else {
        setLabel(tr("Error"));
        QMessageBox::critical(this, tr("Update Error"), "Unable to open the installer. You might be able to manually update"
                " by closing Cockatrice and running the installer at " + filepath.toLocalFile() + ".");
    }
}

void DlgUpdate::downloadProgressMade(qint64 bytesRead, qint64 totalBytes) {
    progress->setMaximum(totalBytes);
    progress->setValue(bytesRead);
}
