#include "filter_card.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QSharedPointer>
#include <iterator>

namespace
{
template <typename Enum> struct FilterIdentifier
{
    Enum value;
    const char *id;
    const char *legacyName;
};

// The names shown in the UI are translated; persisted identities must not be.
// Explicit enum values also keep the format stable when attributes are inserted.
constexpr FilterIdentifier<CardFilter::Type> typeIdentifiers[] = {
    {CardFilter::TypeAnd, "and", "AND"},
    {CardFilter::TypeOr, "or", "OR"},
    {CardFilter::TypeAndNot, "and-not", "AND NOT"},
    {CardFilter::TypeOrNot, "or-not", "OR NOT"},
};
constexpr FilterIdentifier<CardFilter::Attr> attrIdentifiers[] = {
    {CardFilter::AttrCmc, "mana-value", "Mana Value"},
    {CardFilter::AttrColor, "color", "Color"},
    {CardFilter::AttrColorIdentity, "color-identity", "Color Identity"},
    {CardFilter::AttrLoyalty, "loyalty", "Loyalty"},
    {CardFilter::AttrManaCost, "mana-cost", "Mana Cost"},
    {CardFilter::AttrName, "name", "Name"},
    {CardFilter::AttrNameExact, "name-exact", "Name (Exact)"},
    {CardFilter::AttrPow, "power", "Power"},
    {CardFilter::AttrRarity, "rarity", "Rarity"},
    {CardFilter::AttrSet, "set", "Set"},
    {CardFilter::AttrText, "text", "Text"},
    {CardFilter::AttrTough, "toughness", "Toughness"},
    {CardFilter::AttrType, "type", "Type"},
    {CardFilter::AttrMainType, "main-type", "Main Type"},
    {CardFilter::AttrSubType, "sub-type", "Sub Type"},
    {CardFilter::AttrFormat, "format", "Format"},
    {CardFilter::AttrTag, "tag", "Tag"},
};
static_assert(std::size(typeIdentifiers) == CardFilter::TypeEnd);
static_assert(std::size(attrIdentifiers) == CardFilter::AttrEnd);
} // namespace

QJsonObject CardFilter::toJson() const
{
    QJsonObject obj;
    obj["term"] = trm;
    obj["type"] = typeName(t);
    obj["attr"] = attrName(a);
    // Retain the labels for older readers in the language used to save the file.
    for (const auto &identifier : typeIdentifiers) {
        if (identifier.value == t) {
            obj["typeId"] = QLatin1String(identifier.id);
            break;
        }
    }
    for (const auto &identifier : attrIdentifiers) {
        if (identifier.value == a) {
            obj["attrId"] = QLatin1String(identifier.id);
            break;
        }
    }
    return obj;
}

CardFilter *CardFilter::fromJson(const QJsonObject &obj)
{
    if (!obj["term"].isString()) {
        return nullptr;
    }
    QString term = obj["term"].toString();
    const bool hasIds = obj.contains("typeId") || obj.contains("attrId");
    const QJsonValue typeValue = obj[hasIds ? "typeId" : "type"];
    const QJsonValue attrValue = obj[hasIds ? "attrId" : "attr"];
    if (!typeValue.isString() || !attrValue.isString()) {
        return nullptr;
    }
    const QString typeStr = typeValue.toString();
    const QString attrStr = attrValue.toString();

    Type type = TypeEnd;
    Attr attr = AttrEnd;

    // Existing files contain only labels. Accept the current UI's labels and
    // untranslated English labels, without guessing an unknown locale's meaning.
    // Once either ID is present, both must be valid: a malformed ID must never
    // silently fall back to a label with different semantics.
    for (const auto &identifier : typeIdentifiers) {
        if (hasIds ? typeStr == QLatin1String(identifier.id)
                   : typeStr == typeName(identifier.value) || typeStr == QLatin1String(identifier.legacyName)) {
            type = identifier.value;
            break;
        }
    }

    for (const auto &identifier : attrIdentifiers) {
        if (hasIds ? attrStr == QLatin1String(identifier.id)
                   : attrStr == attrName(identifier.value) || attrStr == QLatin1String(identifier.legacyName)) {
            attr = identifier.value;
            break;
        }
    }

    return type == TypeEnd || attr == AttrEnd ? nullptr : new CardFilter(term, type, attr);
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
        case AttrNameExact:
            return tr("Name (Exact)");
        case AttrType:
            return tr("Type");
        case AttrColor:
            return tr("Color");
        case AttrColorIdentity:
            return tr("Color Identity");
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
        case AttrTag:
            return tr("Tag");
        case AttrMainType:
            return tr("Main Type");
        case AttrSubType:
            return tr("Sub Type");
        default:
            return QString("");
    }
}
