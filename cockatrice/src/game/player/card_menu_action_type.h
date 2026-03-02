/**
 * @file card_menu_action_type.h
 * @ingroup GameMenusPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_CARD_MENU_ACTION_TYPE_H
#define COCKATRICE_CARD_MENU_ACTION_TYPE_H

enum CardMenuActionType
{
    cmTap,
    cmUntap,
    cmDoesntUntap,
    cmFlip,
    cmPeek,
    cmClone,
    cmMoveToTopLibrary,
    cmMoveToBottomLibrary,
    cmMoveToHand,
    cmMoveToGraveyard,
    cmMoveToExile
};

#endif // COCKATRICE_CARD_MENU_ACTION_TYPE_H
