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
        default:
            return "";
    }
}
