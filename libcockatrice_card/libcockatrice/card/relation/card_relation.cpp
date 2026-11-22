#include "card_relation.h"

CardRelation CardRelation::copyWithName(const QString &newName) const
{
    CardRelation rel(*this);
    rel.name = newName;
    return rel;
}