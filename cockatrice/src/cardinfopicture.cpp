#include "cardinfopicture.h"

#include <QWidget>
#include <QPainter>
#include <QStyle>

#include "carditem.h"
#include "carddatabase.h"
#include "pictureloader.h"
#include "main.h"

CardInfoPicture::CardInfoPicture(QWidget *parent)
    : QWidget(parent),
    info(0),
    pixmapDirty(true)
{
    setMinimumHeight(100);
}

void CardInfoPicture::setCard(CardInfo *card)
{
    if (info)
        disconnect(info, 0, this, 0);
    info = card;
    connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));

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
    if(info)
        PictureLoader::getPixmap(resizedPixmap, info, size());
    else
        resizedPixmap = QPixmap();


    if (resizedPixmap.isNull())
        PictureLoader::getPixmap(resizedPixmap, db->getCard(), size());
}

void CardInfoPicture::paintEvent(QPaintEvent *)
{
    if (width() == 0 || height() == 0)
        return;

    if(pixmapDirty)
        loadPixmap();

    QPainter painter(this);
    style()->drawItemPixmap(&painter, rect(), Qt::AlignHCenter, resizedPixmap);
}
