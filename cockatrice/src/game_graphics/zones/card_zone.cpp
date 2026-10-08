#include "card_zone.h"

#include "../board/card_item.h"
#include "view_zone.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>

CardZone::CardZone(CardZoneLogic *_logic, QGraphicsItem *parent)
    : AbstractGraphicsItem(parent), menu(nullptr), doubleClickAction(0), logic(_logic)
{
    connect(logic, &CardZoneLogic::retranslateUi, this, &CardZone::retranslateUi);
    // Invalidation is connected before any consumer so a handler that reads cardItems()
    // during the same signal emission always sees a stale flag and rebuilds.
    connect(logic, &CardZoneLogic::cardAdded, this, &CardZone::invalidateItems);
    connect(logic, &CardZoneLogic::cardCountChanged, this, &CardZone::invalidateItems);
    connect(logic, &CardZoneLogic::reorganizeCards, this, &CardZone::invalidateItems);
    connect(logic, &CardZoneLogic::cardAdded, this, &CardZone::onCardAdded);
    connect(logic, &CardZoneLogic::requestCreateCard, this, &CardZone::onCreateCardRequested);
    connect(logic, &CardZoneLogic::setGraphicsVisibility, this, [this](bool v) { this->setVisible(v); });
    connect(logic, &CardZoneLogic::updateGraphics, this, [this]() { update(); });
    connect(logic, &CardZoneLogic::reorganizeCards, this, &CardZone::reorganizeCards);
}

void CardZone::invalidateItems()
{
    itemsDirty = true;
}

const QList<CardItem *> &CardZone::cardItems() const
{
    if (itemsDirty) {
        items.clear();
        for (auto *cardState : getLogic()->getCards()) {
            if (auto *cardItem = qobject_cast<CardItem *>(cardState->parent())) {
                items.append(cardItem);
            }
        }
        itemsDirty = false;
    }
    return items;
}

void CardZone::onCardAdded(CardState *addedCard)
{
    auto *addedItem = addedCard == nullptr ? nullptr : qobject_cast<CardItem *>(addedCard->parent());
    if (addedItem == nullptr) {
        return;
    }
    addedItem->setParentItem(this);
    addedItem->setVisible(true);
    addedItem->update();
}

void CardZone::onCreateCardRequested(const ServerInfo_Card &cardInfo, bool reorganize)
{
    CardRef cardRef = {QString::fromStdString(cardInfo.name()), QString::fromStdString(cardInfo.provider_id())};
    auto *card = new CardItem(getLogic()->getPlayer(), nullptr, cardRef, cardInfo.id());
    card->processCardInfo(cardInfo);
    getLogic()->addCard(card->getState(), reorganize, cardInfo.x(), cardInfo.y());
}

void CardZone::retranslateUi()
{
    for (auto *card : cardItems()) {
        card->retranslateUi();
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
