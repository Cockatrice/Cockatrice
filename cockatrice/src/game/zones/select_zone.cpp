#include "select_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../board/card_item.h"
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

SelectZone::SelectZone(CardZoneLogic *_logic, QGraphicsItem *parent) : CardZone(_logic, parent)
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
        for (auto card : getLogic()->getCards()) {
            if (card->getAttachedTo() && card->getAttachedTo()->getZone() != getLogic()) {
                continue;
            }

            bool inRect = selectionRect.intersects(card->mapRectToParent(card->boundingRect()));
            if (inRect && !cardsInSelectionRect.contains(card)) {
                // selection has just expanded to cover the card
                cardsInSelectionRect.insert(card);
                card->setSelected(!card->isSelected());
            } else if (!inRect && cardsInSelectionRect.contains(card)) {
                // selection has just shrunk to no longer cover the card
                cardsInSelectionRect.remove(card);
                card->setSelected(!card->isSelected());
            }
        }
        static_cast<GameScene *>(scene())->resizeRubberBand(
            deviceTransform(static_cast<GameScene *>(scene())->getViewportTransform()).map(pos));
        event->accept();
    }
}

void SelectZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!event->modifiers().testFlag(Qt::ControlModifier)) {
            scene()->clearSelection();
        }

        selectionOrigin = event->pos();
        static_cast<GameScene *>(scene())->startRubberBand(event->scenePos());
        event->accept();
    } else
        CardZone::mousePressEvent(event);
}

void SelectZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    selectionOrigin = QPoint();
    cardsInSelectionRect.clear();
    static_cast<GameScene *>(scene())->stopRubberBand();
    event->accept();
}
