#include "deck_list_card_node.h"

#include "libcockatrice/deck_list/tree/abstract_deck_list_card_node.h"

class InnerDecklistNode;

DecklistCardNode::DecklistCardNode(DecklistCardNode *other, InnerDecklistNode *_parent)
    : AbstractDecklistCardNode(_parent), name(other->getName()), number(other->getNumber()),
      cardSetShortName(other->getCardSetShortName()), cardSetNumber(other->getCardCollectorNumber()),
      cardProviderId(other->getCardProviderId())
{
}