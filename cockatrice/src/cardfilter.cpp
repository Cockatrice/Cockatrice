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
        default:
            return "";
    }
}
