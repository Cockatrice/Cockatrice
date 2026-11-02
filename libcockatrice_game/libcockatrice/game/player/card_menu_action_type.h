/**
 * @file card_menu_action_type.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

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
    cmMoveToTable,
    // Continuumatrice piles: append to preserve existing action values.
    cmMoveToLimbo,
    cmMoveToRefuge,
    cmMoveToOblivion
};

#endif // COCKATRICE_CARD_MENU_ACTION_TYPE_H
