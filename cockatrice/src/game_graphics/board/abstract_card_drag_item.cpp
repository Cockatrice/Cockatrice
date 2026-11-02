#include "abstract_card_drag_item.h"

#include "../../client/settings/cache_settings.h"
#include "../z_values.h"
#include "abstract_card_item.h"

#include <QBrush>
#include <QColor>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <QtGlobal>
#include <libcockatrice/settings/cards_display_settings.h>
#include <qnamespace.h>

class QPointF;

const QColor GHOST_MASK = QColor(255, 255, 255, 50);

AbstractCardDragItem::AbstractCardDragItem(AbstractCardItem *_item,
                                           const QPointF &_hotSpot,
                                           AbstractCardDragItem *parentDrag)
    : QGraphicsItem(), item(_item), hotSpot(_hotSpot)
{
    if (parentDrag) {
        parentDrag->addChildDrag(this);
        setZValue(ZValues::childDragZValue(hotSpot.x(), hotSpot.y()));
        connect(parentDrag, &QObject::destroyed, this, &AbstractCardDragItem::deleteLater);
    } else {
        hotSpot = QPointF{qBound(0.0, hotSpot.x(), CardDimensions::WIDTH_F - 1),
                          qBound(0.0, hotSpot.y(), CardDimensions::HEIGHT_F - 1)};
        setCursor(Qt::ClosedHandCursor);
        setZValue(ZValues::DRAG_ITEM);
    }
    if (item->getTapped()) {
        setTransform(QTransform()
                         .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                         .rotate(90)
                         .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));
    }

    setCacheMode(DeviceCoordinateCache);

    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::roundCardCornersChanged, this,
            [this](bool _roundCardCorners) {
                Q_UNUSED(_roundCardCorners);

                prepareGeometryChange();
                update();
            });

    // The source card can be destroyed mid-drag (e.g. moved by a server event). deleteLater() alone is not enough:
    // a scene repaint can run before the deferred delete and paint the dead card. Detach from the scene immediately.
    connect(_item, &QObject::destroyed, this, [this]() {
        hide();
        if (QGraphicsScene *sc = scene()) {
            sc->removeItem(this);
        }
        deleteLater();
    });
}

AbstractCardItem *AbstractCardDragItem::getItem() const
{
    return item.data();
}

QPainterPath AbstractCardDragItem::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius =
        SettingsCache::instance().cardsDisplay().getRoundCardCorners() ? 0.05 * CardDimensions::WIDTH_F : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void AbstractCardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (item.isNull()) {
        return;
    }

    item->paint(painter, option, widget);

    // adds a mask to the card so it looks like the card hasnt been placed yet
    painter->fillPath(shape(), GHOST_MASK);
}

void AbstractCardDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    updatePosition(event->scenePos());
}

void AbstractCardDragItem::addChildDrag(AbstractCardDragItem *child)
{
    childDrags << child;
}
