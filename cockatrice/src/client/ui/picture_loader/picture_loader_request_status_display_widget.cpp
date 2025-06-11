#include "picture_loader_request_status_display_widget.h"

PictureLoaderRequestStatusDisplayWidget::PictureLoaderRequestStatusDisplayWidget(QWidget *parent,
                                                                                 const QUrl &_url,
                                                                                 PictureLoaderWorkerWork *worker)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);

    name = new QLabel(this);
    name->setText(worker->cardToDownload.getCard()->getName());
    setShortname = new QLabel(this);
    setShortname->setText(worker->cardToDownload.getSetName());
    providerId = new QLabel(this);
    providerId->setText(worker->cardToDownload.getCard()->getProperty("uuid"));
    startTime = new QLabel(this);
    startTime->setText(QDateTime::currentDateTime().toString());
    elapsedTime = new QLabel(this);
    elapsedTime->setText("0");
    finished = new QLabel(this);
    finished->setText("False");
    url = new QLabel(this);
    url->setText(_url.toString());

    layout->addWidget(name);
    layout->addWidget(setShortname);
    layout->addWidget(providerId);
    layout->addWidget(startTime);
    layout->addWidget(elapsedTime);
    layout->addWidget(finished);
    layout->addWidget(url);
}