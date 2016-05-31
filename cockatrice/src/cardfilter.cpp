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
            return "name";
        case AttrType:
            return "type";
        case AttrColor:
            return "color";
        case AttrText:
            return "text";
        case AttrSet:
            return "set";
        case AttrManaCost:
            return "mana cost";
        case AttrCmc:
            return "cmc";
        case AttrRarity:
            return "rarity";
        default:
            return "";
    }
}
