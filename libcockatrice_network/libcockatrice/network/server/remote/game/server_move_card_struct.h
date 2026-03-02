#ifndef MOVE_CARD_STRUCT
#define MOVE_CARD_STRUCT

#include "server_card.h"
class CardToMove;

struct MoveCardStruct
{
    Server_Card *card;
    int position;
    const CardToMove *cardToMove;
    int xCoord, yCoord;
    MoveCardStruct(Server_Card *_card, int _position, const CardToMove *_cardToMove)
        : card(_card), position(_position), cardToMove(_cardToMove), xCoord(_card->getX()), yCoord(_card->getY())

    {
    }
    bool operator<(const MoveCardStruct &other) const
    {
        return (yCoord == other.yCoord &&
                ((xCoord == other.xCoord && position < other.position) || xCoord < other.xCoord)) ||
               yCoord < other.yCoord;
    }
};

#endif
