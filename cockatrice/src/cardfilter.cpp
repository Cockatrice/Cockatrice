#include "cardfilter.h"

const QString CardFilter::typeName(Type t)
{
    switch (t) {
        case TypeAnd:
            return tr("AND", "Logical conjunction operator used in card filter");
        case TypeOr:
            return tr("OR", "Logical disjunction operator used in card filter");
        case TypeAndNot:
            return tr("AND NOT", "Negated logical conjunction operator used in card filter");
        case TypeOrNot:
            return tr("OR NOT", "Negated logical disjunction operator used in card filter");
        default:
            return QString("");
    }
}

const QString CardFilter::attrName(Attr a)
{
    switch (a) {
        case AttrName:
            return tr("Name");
        case AttrType:
            return tr("Type");
        case AttrColor:
            return tr("Color");
        case AttrText:
            return tr("Text");
        case AttrSet:
            return tr("Set");
        case AttrManaCost:
            return tr("Mana Cost");
        case AttrCmc:
            return tr("CMC");
        case AttrRarity:
            return tr("Rarity");
        case AttrPow:
            return tr("Power");
        case AttrTough:
            return tr("Toughness");
        case AttrLoyalty:
            return tr("Loyalty");
        default:
            return QString("");
    }
}
