/**
 * @file card_move_request.h
 * @ingroup GameLogicActions
 */
//! \todo Document this file.

#ifndef COCKATRICE_CARD_MOVE_REQUEST_H
#define COCKATRICE_CARD_MOVE_REQUEST_H

#include <QString>

/**
 * @brief Describes one card a view wants moved, in logic terms rather than wire terms.
 */
struct CardMoveRequest
{
    int cardId = -1;
    bool faceDown = false;
    QString pt = {};
};

#endif // COCKATRICE_CARD_MOVE_REQUEST_H
