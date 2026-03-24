/**
 * @file card_menu_action_type.h
 * @ingroup GameMenusPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_CARD_MENU_ACTION_TYPE_H
#define COCKATRICE_CARD_MENU_ACTION_TYPE_H

enum CardMenuActionType
{
    // Per-card attribute actions (must be <= cmClone for cardMenuAction() dispatch)
    cmTap,
    cmUntap,
    cmDoesntUntap,
    cmFlip,
    cmPeek,
    cmClone,
    // Move actions (must be > cmClone for cardMenuAction() dispatch)
    cmMoveToTopLibrary,
    cmMoveToBottomLibrary,
    cmMoveToHand,
    cmMoveToGraveyard,
    cmMoveToExile,
    cmMoveToTable
};

#endif // COCKATRICE_CARD_MENU_ACTION_TYPE_H
