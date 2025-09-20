#include "counter_general.h"

#include "../../interface/pixel_map_generator.h"
#include "abstract_graphics_item.h"

#include <QPainter>

GeneralCounter::GeneralCounter(Player *_player,
                               int _id,
                               const QString &_name,
                               const QColor &_color,
                               int _radius,
                               int _value,
                               bool useNameForShortcut,
                               QGraphicsItem *parent)
    : AbstractCounter(_player, _id, _name, true, _value, useNameForShortcut, parent), color(_color), radius(_radius)
{
    setCacheMode(DeviceCoordinateCache);
}

QRectF GeneralCounter::boundingRect() const
{
    return QRectF(0, 0, radius * 2, radius * 2);
}

void GeneralCounter::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QRectF mapRect = painter->combinedTransform().mapRect(boundingRect());
    int translatedHeight = mapRect.size().height();
    qreal scaleFactor = translatedHeight / boundingRect().height();
    QPixmap pixmap = CounterPixmapGenerator::generatePixmap(translatedHeight, name, hovered);

    painter->save();
    resetPainterTransform(painter);
    painter->drawPixmap(QPoint(0, 0), pixmap);

    if (value) {
        QFont f("Serif");
        f.setPixelSize(qMax((int)(radius * scaleFactor), 10));
        f.setWeight(QFont::Bold);
        painter->setPen(Qt::black);
        painter->setFont(f);
        painter->drawText(mapRect, Qt::AlignCenter, QString::number(value));
    }
    painter->restore();
}
