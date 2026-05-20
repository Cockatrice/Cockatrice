#include "counter_general.h"

#include "../../game_graphics/board/abstract_graphics_item.h"
#include "../../interface/pixel_map_generator.h"

#include <QPainter>

GeneralCounter::GeneralCounter(CounterState *state, PlayerLogic *player, bool useNameForShortcut, QGraphicsItem *parent)
    : AbstractCounter(state, player, true, useNameForShortcut, parent)
{
    setCacheMode(DeviceCoordinateCache);
}

QRectF GeneralCounter::boundingRect() const
{
    return QRectF(0, 0, state->getRadius() * 2, state->getRadius() * 2);
}

void GeneralCounter::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QRectF mapRect = painter->combinedTransform().mapRect(boundingRect());
    int translatedHeight = mapRect.size().height();
    qreal scaleFactor = translatedHeight / boundingRect().height();
    QPixmap pixmap = CounterPixmapGenerator::generatePixmap(translatedHeight, state->getName(), hovered);

    painter->save();
    resetPainterTransform(painter);
    painter->drawPixmap(QPoint(0, 0), pixmap);

    if (state->getValue()) {
        QFont f("Serif");
        f.setPixelSize(qMax((int)(state->getRadius() * scaleFactor), 10));
        f.setWeight(QFont::Bold);
        painter->setPen(Qt::black);
        painter->setFont(f);
        painter->drawText(mapRect, Qt::AlignCenter, QString::number(state->getValue()));
    }
    painter->restore();
}
