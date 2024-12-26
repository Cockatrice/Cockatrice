#include "select_zone.h"

#include "../../settings/cache_settings.h"
#include "../cards/card_item.h"
#include "../game_scene.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>

qreal divideCardSpaceInZone(qreal index, int cardCount, qreal totalHeight, qreal cardHeight, bool reverse)
{
    qreal cardMinOverlap = cardHeight * SettingsCache::instance().getStackCardOverlapPercent() / 100;
    qreal desiredHeight = cardHeight * cardCount - cardMinOverlap * (cardCount - 1);
    qreal y;
    if (desiredHeight > totalHeight) {
        if (reverse) {
            y = index / ((totalHeight - cardHeight) / (cardCount - 1));
        } else {
            y = index * (totalHeight - cardHeight) / (cardCount - 1);
        }
    } else {
        qreal start = (totalHeight - desiredHeight) / 2;
        if (reverse) {
            if (index <= start) {
                return 0;
            }
            y = (index - start) / (cardHeight - cardMinOverlap);
        } else {
            y = index * (cardHeight - cardMinOverlap) + start;
        }
    }
    return y;
}

SelectZone::SelectZone(Player *_player,
                       const QString &_name,
                       bool _hasCardAttr,
                       bool _isShufflable,
                       bool _contentsKnown,
                       QGraphicsItem *parent,
                       bool isView)
    : CardZone(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent, isView)
{
}

void SelectZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        QPointF pos = event->pos();
        if (pos.x() < 0)
            pos.setX(0);
        QRectF br = boundingRect();
        if (pos.x() > br.width())
            pos.setX(br.width());
        if (pos.y() < 0)
            pos.setY(0);
        if (pos.y() > br.height())
            pos.setY(br.height());

        QRectF selectionRect = QRectF(selectionOrigin, pos).normalized();
        for (auto card : cards) {
            if (card->getAttachedTo() && card->getAttachedTo()->getZone() != this) {
                continue;
            }
            card->setSelected(selectionRect.intersects(card->mapRectToParent(card->boundingRect())));
        }
        static_cast<GameScene *>(scene())->resizeRubberBand(
            deviceTransform(static_cast<GameScene *>(scene())->getViewportTransform()).map(pos));
        event->accept();
    }
}

void SelectZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scene()->clearSelection();

        selectionOrigin = event->pos();
        static_cast<GameScene *>(scene())->startRubberBand(event->scenePos());
        event->accept();
    } else
        CardZone::mousePressEvent(event);
}

void SelectZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    selectionOrigin = QPoint();
    static_cast<GameScene *>(scene())->stopRubberBand();
    event->accept();
}
