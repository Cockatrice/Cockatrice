#include "cardinfopicture.h"

#include <QLabel>
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardInfoPicture::CardInfoPicture(QWidget *parent)
    : QLabel(parent)
    , info(0)
{
    setAlignment(Qt::AlignCenter);
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
        return;
    }

    QPixmap resizedPixmap;
    info->getPixmap(size(), resizedPixmap);

    if (resizedPixmap.isNull()) {
        db->getCard()->getPixmap(size(), resizedPixmap);
    }

    setPixmap(resizedPixmap);
}
