/**
 * @file move_top_cards_until_options.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_MOVE_TOP_CARDS_UNTIL_OPTIONS_H
#define COCKATRICE_MOVE_TOP_CARDS_UNTIL_OPTIONS_H

#include <QStringList>

struct MoveTopCardsUntilOptions
{
    QStringList exprs = {};
    int numberOfHits = 1;
    bool autoPlay = false;
};

#endif // COCKATRICE_MOVE_TOP_CARDS_UNTIL_OPTIONS_H
