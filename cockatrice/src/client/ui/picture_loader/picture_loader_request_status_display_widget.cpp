#include "picture_loader_request_status_display_widget.h"

PictureLoaderRequestStatusDisplayWidget::PictureLoaderRequestStatusDisplayWidget(QWidget *parent,
                                                                                 const QUrl &_url,
                                                                                 PictureLoaderWorkerWork *worker)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);

    if (worker->cardToDownload.getCard()) {
        name = new QLabel(this);
        name->setText(worker->cardToDownload.getCard()->getName());
        setShortname = new QLabel(this);
        setShortname->setText(worker->cardToDownload.getSetName());
        providerId = new QLabel(this);
        providerId->setText(worker->cardToDownload.getCard()->getProperty("uuid"));

        layout->addWidget(name);
        layout->addWidget(setShortname);
        layout->addWidget(providerId);
    }

    startTime = new QLabel(QDateTime::currentDateTime().toString(), this);
    elapsedTime = new QLabel("0", this);
    finished = new QLabel("False", this);
    url = new QLabel(_url.toString(), this);

    layout->addWidget(startTime);
    layout->addWidget(elapsedTime);
    layout->addWidget(finished);
    layout->addWidget(url);
}