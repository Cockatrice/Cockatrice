#include "picture_loader_status_bar.h"

#include "picture_loader_request_status_display_widget.h"

PictureLoaderStatusBar::PictureLoaderStatusBar(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    progressBar = new QProgressBar(this);
    progressBar->setMaximum(0);
    progressBar->setFormat("%v/%m");
    layout->addWidget(progressBar);

    loadLog = new SettingsButtonWidget(this);
    layout->addWidget(loadLog);

    cleaner = new QTimer(this);
    cleaner->setInterval(1000);
    connect(cleaner, &QTimer::timeout, this, &PictureLoaderStatusBar::cleanOldEntries);
    cleaner->start();

    setLayout(layout);
}

void PictureLoaderStatusBar::cleanOldEntries()
{
    if (!loadLog || !loadLog->popup) {
        return;
    }
    for (PictureLoaderRequestStatusDisplayWidget *statusDisplayWidget :
         loadLog->popup->findChildren<PictureLoaderRequestStatusDisplayWidget *>()) {
        statusDisplayWidget->queryElapsedSeconds();
        if (statusDisplayWidget->getFinished() &&
            QDateTime::fromString(statusDisplayWidget->getStartTime()).secsTo(QDateTime::currentDateTime()) > 10) {
            loadLog->removeSettingsWidget(statusDisplayWidget);
            progressBar->setMaximum(progressBar->maximum() - 1);
            progressBar->setValue(progressBar->value() - 1);
        }
    }
}

void PictureLoaderStatusBar::addQueuedImageLoad(const QUrl &url, const ExactCard &card, const QString &setName)
{
    loadLog->addSettingsWidget(new PictureLoaderRequestStatusDisplayWidget(loadLog, url, card, setName));
    progressBar->setMaximum(progressBar->maximum() + 1);
}

void PictureLoaderStatusBar::addSuccessfulImageLoad(const QUrl &url)
{
    progressBar->setValue(progressBar->value() + 1);
    for (PictureLoaderRequestStatusDisplayWidget *statusDisplayWidget :
         loadLog->popup->findChildren<PictureLoaderRequestStatusDisplayWidget *>()) {
        if (statusDisplayWidget->getUrl() == url.toString()) {
            statusDisplayWidget->queryElapsedSeconds();
            statusDisplayWidget->setFinished();
        }
    }
}