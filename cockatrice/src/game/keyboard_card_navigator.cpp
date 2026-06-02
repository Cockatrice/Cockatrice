#include "keyboard_card_navigator.h"

#include "../../client/settings/cache_settings.h"
#include "board/arrow_item.h"
#include "board/card_item.h"
#include "player/player_logic.h"

#include <QApplication>
#include <QKeyEvent>
KeyboardCardNavigator::KeyboardCardNavigator(PlayerLogic *player)
    : currentZone(nullptr), hoveredCardIndex(-1), isArrowModeActive(false), arrowOriginCard(nullptr),
      previewArrow(nullptr), playerLogic(player)
{
}

int KeyboardCardNavigator::getHoveredIndex()
{
    return hoveredCardIndex;
}

CardZoneLogic *KeyboardCardNavigator::getCurrentZone()
{
    return currentZone;
}

void KeyboardCardNavigator::setCurrentZone(CardZoneLogic *zone)
{
    currentZone = zone;
}

void KeyboardCardNavigator::switchCardInZone(QKeyEvent *event)
{
    if (!playerLogic) {
        return;
    }
    if (!currentZone) {
        return;
    }
    if (QApplication::activePopupWidget()) {
        return;
    }

    const CardList &zoneCards = currentZone->getCards();
    if (zoneCards.isEmpty()) {
        // if the current zone is empty, try to force a zone change.
        QKeyEvent event(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        KeyboardCardNavigator::switchZone(&event);
        return;
    }
    event->accept();

    // Check if this is an arrow key we care about
    int keyCode = event->key();
    if (keyCode != Qt::Key_Right && keyCode != Qt::Key_Left) {
        return;
    }

    // Calculate new index
    int newIndex = hoveredCardIndex;
    bool isInitial = (hoveredCardIndex < 0);

    if (isInitial) {
        // If the first key pressed is the right, spawn the cursor at the first card
        // Otherwise, spawn at the last card.
        newIndex = keyCode == Qt::Key_Right ? 0 : zoneCards.size() - 1;
    } else {
        if (hoveredCardIndex >= zoneCards.size()) {
            hoveredCardIndex = 0;
            newIndex = 0;
        }

        if (keyCode == Qt::Key_Right) {
            newIndex = (hoveredCardIndex + 1) % zoneCards.size();
        } else {
            newIndex = (hoveredCardIndex - 1 + zoneCards.size()) % zoneCards.size();
        }
    }

    KeyboardCardNavigator::changeHoverCard(hoveredCardIndex, false);

    hoveredCardIndex = newIndex;
    KeyboardCardNavigator::changeHoverCard(newIndex, true);
}

void KeyboardCardNavigator::changeHoverCard(int cardIndex, bool hover)
{
    const CardList &zoneCards = currentZone->getCards();
    if (cardIndex >= 0 && cardIndex < zoneCards.size()) {
        CardItem *card = zoneCards[cardIndex];
        if (card) {
            card->setHovered(hover);
            card->setFocus();
            // Force update of new card's area
            if (card->scene()) {
                card->scene()->update(card->sceneBoundingRect());
            }
            if (isArrowModeActive && hover) {
                createTempArrow(card);
            }
        }
    }
}

void KeyboardCardNavigator::setPlayer(PlayerLogic *player)
{
    playerLogic = player;
}

void KeyboardCardNavigator::setHoveredCardIndex(int index)
{
    hoveredCardIndex = index;
}

void KeyboardCardNavigator::unhoverCard()
{
    if (!playerLogic || !currentZone) {
        return;
    }

    changeHoverCard(hoveredCardIndex, false);
}

void KeyboardCardNavigator::createTempArrow(CardItem *targetCard)
{
    if (!isArrowModeActive || !arrowOriginCard || !targetCard || !playerLogic) {
        return;
    }

    if (previewArrow) {
        delete previewArrow;
        previewArrow = nullptr;
    }

    previewArrow = new ArrowItem(playerLogic, -1, arrowOriginCard, targetCard, Qt::red);
    if (arrowOriginCard->scene()) {
        arrowOriginCard->scene()->addItem(previewArrow);
    }
}
void KeyboardCardNavigator::createArrow(CardItem *targetCard)
{
    if (!isArrowModeActive || !arrowOriginCard || !targetCard || !playerLogic) {
        return;
    }

    if (previewArrow) {
        delete previewArrow;
        previewArrow = nullptr;
    }
    isArrowModeActive = false;

    if (arrowOriginCard == targetCard) {
        arrowOriginCard = nullptr;
        return;
    }

    ArrowItem::sendCreateArrowCommand(playerLogic, arrowOriginCard, targetCard, Qt::red);

    arrowOriginCard = nullptr;
}

void KeyboardCardNavigator::startArrowMode(CardItem *originCard)
{
    if (!originCard || !originCard->scene() || !playerLogic) {
        return;
    }

    isArrowModeActive = true;
    arrowOriginCard = originCard;

    createTempArrow(originCard);
}

void KeyboardCardNavigator::cancelArrowMode()
{
    if (previewArrow) {
        delete previewArrow;
        previewArrow = nullptr;
    }
    isArrowModeActive = false;
    arrowOriginCard = nullptr;
}

CardZoneLogic *
KeyboardCardNavigator::findZoneWithCards(QList<CardZoneLogic *> &zonesList, int currentZoneIndex, bool upperZone)
{
    CardZoneLogic *newZone;
    int newZoneIndex = currentZoneIndex;
    do {
        // Calculate new zone index
        if (upperZone) {
            newZoneIndex = (newZoneIndex + 1) % zonesList.size();
        } else {
            newZoneIndex = (newZoneIndex - 1 + zonesList.size()) % zonesList.size();
        }
        newZone = zonesList[newZoneIndex];
        // Prevent switching zone if the others are empty
    } while (newZoneIndex != currentZoneIndex && newZone->getCards().size() == 0);
    return newZone;
}

void KeyboardCardNavigator::switchZone(QKeyEvent *event)
{
    if (!playerLogic) {
        return;
    }

    if (QApplication::activePopupWidget()) {
        return;
    }

    int keyCode = event->key();
    if (keyCode != Qt::Key_Up && keyCode != Qt::Key_Down) {
        return;
    }

    event->accept();
    // Build list with only the zones of interest
    QList<CardZoneLogic *> zonesList;

    TableZoneLogic *tableZone = playerLogic->getTableZone();
    StackZoneLogic *stackZone = playerLogic->getStackZone();
    HandZoneLogic *handZone = playerLogic->getHandZone();

    if (tableZone) {
        zonesList.append(tableZone);
    }
    if (stackZone) {
        zonesList.append(stackZone);
    }
    if (handZone) {
        zonesList.append(handZone);
    }

    if (zonesList.isEmpty()) {
        return;
    }

    int currentZoneIndex = zonesList.indexOf(currentZone);
    if (currentZoneIndex < 0) {

        currentZoneIndex = 0;
    }

    CardZoneLogic *newZone =
        KeyboardCardNavigator::findZoneWithCards(zonesList, currentZoneIndex, keyCode == Qt::Key_Up);

    if (currentZone != newZone) {

        changeHoverCard(hoveredCardIndex, false);
        setCurrentZone(newZone);
        // Reset card index since we're in a new zone
        hoveredCardIndex = 0;
        // The new zone has to have cards, hover and select the first one
        changeHoverCard(hoveredCardIndex, true);
    }
}
