#include "deck_list_card_node.h"

DecklistCardNode::DecklistCardNode(DecklistCardNode *other, InnerDecklistNode *_parent)
    : AbstractDecklistCardNode(_parent), name(other->getName()), number(other->getNumber()),
      cardSetShortName(other->getCardSetShortName()), cardSetNumber(other->getCardCollectorNumber()),
      cardProviderId(other->getCardProviderId())
{
}