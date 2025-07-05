#include "picture_loader_request_status_display_widget.h"

PictureLoaderRequestStatusDisplayWidget::PictureLoaderRequestStatusDisplayWidget(QWidget *parent,
                                                                                 const QUrl &_url,
                                                                                 const CardInfoPtr &card,
                                                                                 const QString &setName)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);

    name = new QLabel(this);
    name->setText(card->getName());
    setShortname = new QLabel(this);
    setShortname->setText(setName);
    providerId = new QLabel(this);
    providerId->setText(card->getProperty("uuid"));

    layout->addWidget(name);
    layout->addWidget(setShortname);
    layout->addWidget(providerId);

    startTime = new QLabel(QDateTime::currentDateTime().toString(), this);
    elapsedTime = new QLabel("0", this);
    finished = new QLabel("False", this);
    url = new QLabel(_url.toString(), this);

    layout->addWidget(startTime);
    layout->addWidget(elapsedTime);
    layout->addWidget(finished);
    layout->addWidget(url);
}