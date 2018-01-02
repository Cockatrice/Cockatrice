#include "cardfilter.h"

const char *CardFilter::typeName(Type t)
{
    switch (t) {
        case TypeAnd:
            return "and";
        case TypeOr:
            return "or";
        case TypeAndNot:
            return "and not";
        case TypeOrNot:
            return "or not";
        default:
            return "";
    }
}

const char *CardFilter::attrName(Attr a)
{
    switch (a) {
        case AttrCmc:
            return "CMC";
        case AttrColor:
            return "Color";
        case AttrManaCost:
            return "Mana Cost";
        case AttrName:
            return "Name";
        case AttrPow:
            return "Power";
        case AttrRarity:
            return "Rarity";
        case AttrSet:
            return "Set";
        case AttrText:
            return "Text";
        case AttrTough:
            return "Toughness";
        case AttrType:
            return "Type";
        default:
            return "";
    }
}
