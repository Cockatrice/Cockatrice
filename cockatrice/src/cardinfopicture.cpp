#include "cardinfopicture.h"

#include <QPainter>
#include <QStyle>
#include <QWidget>

#include "carddatabase.h"
#include "carditem.h"
#include "main.h"
#include "pictureloader.h"

CardInfoPicture::CardInfoPicture(QWidget *parent) : QWidget(parent), info(nullptr), pixmapDirty(true)
{
    setMinimumHeight(100);
}

void CardInfoPicture::setCard(CardInfo *card)
{
    if (info)
    {
        disconnect(info, nullptr, this, nullptr);
    }

    info = card;

    if (info)
    {
        connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));
    }

    updatePixmap();
}

void CardInfoPicture::resizeEvent(QResizeEvent *)
{
    updatePixmap();
}

void CardInfoPicture::updatePixmap()
{
    pixmapDirty = true;
    update();
}

void CardInfoPicture::loadPixmap()
{
    if (info)
        PictureLoader::getPixmap(resizedPixmap, info, size());
    else
        PictureLoader::getCardBackPixmap(resizedPixmap, size());
}

void CardInfoPicture::paintEvent(QPaintEvent *)
{
    if (width() == 0 || height() == 0)
        return;

    if (pixmapDirty)
        loadPixmap();

    QPainter painter(this);
    style()->drawItemPixmap(&painter, rect(), Qt::AlignHCenter, resizedPixmap);
}
