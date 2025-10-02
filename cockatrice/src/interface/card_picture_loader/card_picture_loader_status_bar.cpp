#include "card_picture_loader_status_bar.h"

#include "card_picture_loader_request_status_display_widget.h"

CardPictureLoaderStatusBar::CardPictureLoaderStatusBar(QWidget *parent) : QWidget(parent)
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
    connect(cleaner, &QTimer::timeout, this, &CardPictureLoaderStatusBar::cleanOldEntries);
    cleaner->start();

    setLayout(layout);
}

void CardPictureLoaderStatusBar::cleanOldEntries()
{
    if (!loadLog || !loadLog->popup) {
        return;
    }
    for (CardPictureLoaderRequestStatusDisplayWidget *statusDisplayWidget :
         loadLog->popup->findChildren<CardPictureLoaderRequestStatusDisplayWidget *>()) {
        statusDisplayWidget->queryElapsedSeconds();
        if (statusDisplayWidget->getFinished() &&
            QDateTime::fromString(statusDisplayWidget->getStartTime()).secsTo(QDateTime::currentDateTime()) > 10) {
            loadLog->removeSettingsWidget(statusDisplayWidget);
            progressBar->setMaximum(progressBar->maximum() - 1);
            progressBar->setValue(progressBar->value() - 1);
        }
    }
}

void CardPictureLoaderStatusBar::addQueuedImageLoad(const QUrl &url, const ExactCard &card, const QString &setName)
{
    loadLog->addSettingsWidget(new CardPictureLoaderRequestStatusDisplayWidget(loadLog, url, card, setName));
    progressBar->setMaximum(progressBar->maximum() + 1);
}

void CardPictureLoaderStatusBar::addSuccessfulImageLoad(const QUrl &url)
{
    progressBar->setValue(progressBar->value() + 1);
    for (CardPictureLoaderRequestStatusDisplayWidget *statusDisplayWidget :
         loadLog->popup->findChildren<CardPictureLoaderRequestStatusDisplayWidget *>()) {
        if (statusDisplayWidget->getUrl() == url.toString()) {
            statusDisplayWidget->queryElapsedSeconds();
            statusDisplayWidget->setFinished();
        }
    }
}