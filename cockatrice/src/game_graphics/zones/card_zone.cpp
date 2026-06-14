#include "card_zone.h"

#include "../board/card_item.h"
#include "view_zone.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>

CardZone::CardZone(CardZoneLogic *_logic, QGraphicsItem *parent)
    : AbstractGraphicsItem(parent), logic(_logic), menu(nullptr), doubleClickAction(0)
{
    connect(logic, &CardZoneLogic::retranslateUi, this, &CardZone::retranslateUi);
    connect(logic, &CardZoneLogic::cardAdded, this, &CardZone::onCardAdded);
    connect(logic, &CardZoneLogic::cardRemoved, this, &CardZone::onCardRemoved);
    connect(logic, &CardZoneLogic::setGraphicsVisibility, this, [this](bool v) { this->setVisible(v); });
    connect(logic, &CardZoneLogic::updateGraphics, this, [this]() { update(); });
    connect(logic, &CardZoneLogic::reorganizeCards, this, &CardZone::reorganizeCards);
}

void CardZone::onCardAdded(CardState *toAdd, int /*x*/, int /*y*/)
{
    CardItem *addedCard = new CardItem(toAdd, this);
    addedCard->setParentItem(this);
    addedCard->setVisible(true);
    addedCard->update();
    cards.append(addedCard);

    emit cardItemAdded(addedCard);
}

void CardZone::onCardRemoved(CardState *toRemove, int /*x*/, int /*y*/)
{
    CardItem *removedCard = getCardItemForId(toRemove->getId());
    if (!removedCard) {
        return;
    }
    if (cards.contains(removedCard)) {
        cards.remove(cards.indexOf(removedCard));
    }
    removedCard->setVisible(false);
    removedCard->setParentItem(nullptr);
    removedCard->deleteLater();
}

void CardZone::retranslateUi()
{
    for (int i = 0; i < cards.size(); ++i) {
        cards[i]->retranslateUi();
    }
}

void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    if (doubleClickAction) {
        doubleClickAction->trigger();
    }
}

bool CardZone::showContextMenu(const QPoint &screenPos)
{
    if (menu) {
        menu->exec(screenPos);
        return true;
    }
    return false;
}

void CardZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (showContextMenu(event->screenPos())) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

QPointF CardZone::closestGridPoint(const QPointF &point)
{
    return point;
}
