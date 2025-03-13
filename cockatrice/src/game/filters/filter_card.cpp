#include "filter_card.h"

#include <QJsonObject>

QJsonObject CardFilter::toJson() const
{
    QJsonObject obj;
    obj["term"] = trm;
    obj["type"] = typeName(t);
    obj["attr"] = attrName(a);
    return obj;
}

CardFilter *CardFilter::fromJson(const QJsonObject &obj)
{
    QString term = obj["term"].toString();
    QString typeStr = obj["type"].toString();
    QString attrStr = obj["attr"].toString();

    Type type = TypeEnd;
    Attr attr = AttrEnd;

    // Convert type string back to enum
    for (int i = 0; i < TypeEnd; i++) {
        if (typeName(static_cast<Type>(i)) == typeStr) {
            type = static_cast<Type>(i);
            break;
        }
    }

    // Convert attr string back to enum
    for (int i = 0; i < AttrEnd; i++) {
        if (attrName(static_cast<Attr>(i)) == attrStr) {
            attr = static_cast<Attr>(i);
            break;
        }
    }

    return new CardFilter(term, type, attr);
}

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
            return tr("Mana Value");
        case AttrRarity:
            return tr("Rarity");
        case AttrPow:
            return tr("Power");
        case AttrTough:
            return tr("Toughness");
        case AttrLoyalty:
            return tr("Loyalty");
        case AttrFormat:
            return tr("Format");
        case AttrMainType:
            return tr("Main Type");
        case AttrSubType:
            return tr("Sub Type");
        default:
            return QString("");
    }
}
