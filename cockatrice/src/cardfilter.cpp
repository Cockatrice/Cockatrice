#include "cardfilter.h"

const char *CardFilter::typeName(Type t)
{
    switch (t) {
        case TypeAnd:
            return "AND";
        case TypeOr:
            return "OR";
        case TypeAndNot:
            return "AND NOT";
        case TypeOrNot:
            return "OR NOT";
        default:
            return "";
    }
}

const char *CardFilter::attrName(Attr a)
{
    switch (a) {
        case AttrName:
            return "Name";
        case AttrType:
            return "Type";
        case AttrColor:
            return "Color";
        case AttrText:
            return "Text";
        case AttrSet:
            return "Set";
        case AttrManaCost:
            return "Mana Cost";
        case AttrCmc:
            return "CMC";
        case AttrRarity:
            return "Rarity";
        case AttrPow:
            return "Power";
        case AttrTough:
            return "Toughness";
        case AttrLoyalty:
            return "Loyalty";
        default:
            return "";
    }
}
