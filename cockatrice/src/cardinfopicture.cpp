#include "cardinfopicture.h"

#include <QLabel>
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardInfoPicture::CardInfoPicture(int maximumWidth, QWidget *parent)
    : QLabel(parent)
    , info(0)
    , noPicture(true)
{
    setAlignment(Qt::AlignCenter);
    setMaximumWidth(maximumWidth);
}

void CardInfoPicture::setNoPicture(bool status)
{
    if (noPicture != status) {
        noPicture = status;
        emit hasPictureChanged();
    }
}

void CardInfoPicture::setCard(CardInfo *card)
{
    if (info)
        disconnect(info, 0, this, 0);
    info = card;
    connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));

    updatePixmap();
}

void CardInfoPicture::resizeEvent(QResizeEvent * /* e */)
{
    updatePixmap();
}

void CardInfoPicture::updatePixmap()
{
    if (info == 0 || width() == 0 || height() == 0) {
        setNoPicture(true);
        return;
    }

    QPixmap resizedPixmap;
    info->getPixmap(size(), resizedPixmap);

    if (resizedPixmap.isNull()) {
        setNoPicture(true);
        db->getCard()->getPixmap(size(), resizedPixmap);
    } else {
        setNoPicture(false);
    }
    this->setPixmap(resizedPixmap);
}
