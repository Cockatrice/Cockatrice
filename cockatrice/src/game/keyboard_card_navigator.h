/**
 * @file keyboard_card_navigator.h
 * @ingroup GameInput
 * @brief Handles keyboard navigation for selecting cards using arrow keys.
 *
 * Allows players to navigate through hand cards using directional arrow keys.
 * Spatially-aware: finds adjacent cards based on screen position.
 */

#ifndef KEYBOARD_CARD_NAVIGATOR_H
#define KEYBOARD_CARD_NAVIGATOR_H

#include "zones/card_zone_logic.h"

class PlayerLogic;
class CardItem;
class QKeyEvent;
class ArrowItem;

class KeyboardCardNavigator
{
private:
    CardZoneLogic *currentZone;
    int hoveredCardIndex = -1;
    bool isArrowModeActive;
    CardItem *arrowOriginCard;
    ArrowItem *previewArrow;
    PlayerLogic *playerLogic;
    /**
     * @brief Gets hand cards sorted by visual position.
     * @return List of cards sorted by visual order (left-to-right for horizontal, top-to-bottom for vertical).
     */
    CardList getVisuallyOrderedHandCards() const;

public:
    KeyboardCardNavigator(PlayerLogic *player = nullptr);
    int getHoveredIndex();
    CardZoneLogic *getCurrentZone();
    void setCurrentZone(CardZoneLogic *zone);
    void switchCardInZone(QKeyEvent *event);
    void switchZone(QKeyEvent *event);
    void setPlayer(PlayerLogic *player);
    /**
     * @brief Validates and resets the hovered card if needed.
     * Call this when hand composition changes.
     */
    void setHoveredCardIndex(int index);
    void unhoverCard();
    void changeHoverCard(int cardIndex, bool hover);
    void createArrow(CardItem *targetCard);
    void createTempArrow(CardItem *targetCard);
    void startArrowMode(CardItem *originCard);
    void cancelArrowMode();
    CardZoneLogic *findZoneWithCards(QList<CardZoneLogic *> &zonesList, int currentZoneIndex, bool upperZone);
    bool isArrowModeActiveVar() const
    {
        return isArrowModeActive;
    }
};

#endif
