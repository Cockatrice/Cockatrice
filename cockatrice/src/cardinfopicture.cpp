#include "cardinfopicture.h"

#include "carditem.h"
#include "main.h"
#include "pictureloader.h"

#include <QStylePainter>
#include <QWidget>

CardInfoPicture::CardInfoPicture(QWidget *parent) : QWidget(parent), info(nullptr), pixmapDirty(true)
{
    setMinimumHeight(100);
}

void CardInfoPicture::setCard(CardInfoPtr card)
{
    if (info) {
        disconnect(info.data(), nullptr, this, nullptr);
    }

    info = card;

    if (info) {
        connect(info.data(), SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));
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

    QSize scaledSize = resizedPixmap.size().scaled(size(), Qt::KeepAspectRatio);
    QPoint topLeft{(width() - scaledSize.width()) / 2, (height() - scaledSize.height()) / 2};
    qreal radius = 0.05 * scaledSize.width();

    QStylePainter painter(this);
    QPainterPath shape;
    shape.addRoundedRect(QRect(topLeft, scaledSize), radius, radius);
    painter.setClipPath(shape);
    painter.drawItemPixmap(QRect(topLeft, scaledSize), Qt::AlignCenter, resizedPixmap);
}
