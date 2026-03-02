#include "abstract_deck_list_node.h"

#include "inner_deck_list_node.h"

AbstractDecklistNode::AbstractDecklistNode(InnerDecklistNode *_parent, int position)
    : parent(_parent), sortMethod(Default)
{
    if (parent) {
        if (position == -1) {
            parent->append(this);
        } else {
            parent->insert(position, this);
        }
    }
}

int AbstractDecklistNode::depth() const
{
    if (parent) {
        return parent->depth() + 1;
    } else {
        return 0;
    }
}