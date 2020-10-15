#include "pagetemplates.h"

#include "oraclewizard.h"

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QtGui>

SimpleDownloadFilePage::SimpleDownloadFilePage(QWidget *parent) : OracleWizardPage(parent)
{
    urlLabel = new QLabel(this);
    urlLineEdit = new QLineEdit(this);

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlButton = new QPushButton(this);
    connect(urlButton, SIGNAL(clicked()), this, SLOT(actRestoreDefaultUrl()));

    defaultPathCheckBox = new QCheckBox(this);

    pathLabel = new QLabel(this);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *layout = new QGridLayout(this);
    layout->addWidget(urlLabel, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(urlButton, 1, 1, Qt::AlignRight);
    layout->addWidget(pathLabel, 2, 0, 1, 2);
    layout->addWidget(defaultPathCheckBox, 3, 0, 1, 2);
    layout->addWidget(progressLabel, 4, 0);
    layout->addWidget(progressBar, 4, 1);

    setLayout(layout);
}

void SimpleDownloadFilePage::initializePage()
{
    // get custom url from settings if any; otherwise use default url
    urlLineEdit->setText(wizard()->settings->value(getCustomUrlSettingsKey(), getDefaultUrl()).toString());

    progressLabel->hide();
    progressBar->hide();
}

void SimpleDownloadFilePage::actRestoreDefaultUrl()
{
    urlLineEdit->setText(getDefaultUrl());
}

bool SimpleDownloadFilePage::validatePage()
{
    // if data has already been downloaded, pass directly to the "save" step
    if (!downloadData.isEmpty()) {
        if (saveToFile()) {
            return true;
        } else {
            wizard()->enableButtons();
            return false;
        }
    }

    QUrl url = QUrl::fromUserInput(urlLineEdit->text());
    if (!url.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("The provided URL is not valid."));
        return false;
    }

    progressLabel->setText(tr("Downloading (0MB)"));
    // show an infinite progressbar
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    progressBar->setValue(0);
    progressLabel->show();
    progressBar->show();

    wizard()->disableButtons();
    downloadFile(url);
    return false;
}

void SimpleDownloadFilePage::downloadFile(QUrl url)
{
    QNetworkReply *reply = wizard()->nam->get(QNetworkRequest(url));

    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(actDownloadProgress(qint64, qint64)));
}

void SimpleDownloadFilePage::actDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        progressBar->setMaximum(static_cast<int>(total));
        progressBar->setValue(static_cast<int>(received));
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int)received / (1024 * 1024)));
}

void SimpleDownloadFilePage::actDownloadFinished()
{
    // check for a reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));
        wizard()->enableButtons();
        reply->deleteLater();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "following redirect url:" << redirectUrl.toString();
        downloadFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    // save downlaoded file url, but only if the user customized it and download was successfull
    if (urlLineEdit->text() != getDefaultUrl()) {
        wizard()->settings->setValue(getCustomUrlSettingsKey(), urlLineEdit->text());
    } else {
        wizard()->settings->remove(getCustomUrlSettingsKey());
    }

    downloadData = reply->readAll();
    reply->deleteLater();

    wizard()->enableButtons();
    progressLabel->hide();
    progressBar->hide();

    wizard()->next();
}

bool SimpleDownloadFilePage::saveToFile()
{
    QString defaultPath = getDefaultSavePath();
    QString windowName = getWindowTitle();
    QString fileType = getFileType();

    QString fileName;
    if (defaultPathCheckBox->isChecked()) {
        fileName = QFileDialog::getSaveFileName(this, windowName, defaultPath, fileType);
    } else {
        fileName = defaultPath;
    }

    if (fileName.isEmpty()) {
        return false;
    }

    QFileInfo fi(fileName);
    QDir fileDir(fi.path());
    if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
        return false;
    }

    if (!internalSaveToFile(fileName)) {
        QMessageBox::critical(this, tr("Error"), tr("The file could not be saved to %1").arg(fileName));
        return false;
    }

    // clean saved downloadData
    downloadData = QByteArray();
    return true;
}

bool SimpleDownloadFilePage::internalSaveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "File open (w) failed for" << fileName;
        return false;
    }

    if (file.write(downloadData) == -1) {
        qDebug() << "File write (w) failed for" << fileName;
        return false;
    }

    file.close();
    return true;
}
