#include "cardinfopicture.h"

#include <QWidget>
#include <QPainter>
#include <QStyle>

#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardInfoPicture::CardInfoPicture(int width, QWidget *parent)
    : QWidget(parent),
    info(0),
    pixmapDirty(true)
{
    setFixedWidth(width);
    setMinimumHeight(100);
    setMaximumHeight(width / (qreal) CARD_WIDTH * (qreal) CARD_HEIGHT);
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
        info->getPixmap(size(), resizedPixmap);
    else
        resizedPixmap = QPixmap();


    if (resizedPixmap.isNull())
        db->getCard()->getPixmap(size(), resizedPixmap);
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
