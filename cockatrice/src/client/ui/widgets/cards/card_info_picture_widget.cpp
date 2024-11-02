#include "card_info_picture_widget.h"

#include "../../picture_loader.h"
#include "../../../../main.h"
#include "../../../../game/cards/card_item.h"

#include <QStylePainter>
#include <QWidget>

CardInfoPictureWidget::CardInfoPictureWidget(QWidget *parent) : QWidget(parent), info(nullptr), pixmapDirty(true)
{
    setMinimumHeight(100);
}

void CardInfoPictureWidget::setCard(CardInfoPtr card)
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

void CardInfoPictureWidget::resizeEvent(QResizeEvent *)
{
    updatePixmap();
}

void CardInfoPictureWidget::updatePixmap()
{
    pixmapDirty = true;
    update();
}

void CardInfoPictureWidget::loadPixmap()
{
    if (info)
        PictureLoader::getPixmap(resizedPixmap, info, size());
    else
        PictureLoader::getCardBackPixmap(resizedPixmap, size());
}

void CardInfoPictureWidget::paintEvent(QPaintEvent *)
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
