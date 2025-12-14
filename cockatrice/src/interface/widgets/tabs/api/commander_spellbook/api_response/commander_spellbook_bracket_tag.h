#ifndef COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_TAG_H
#define COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_TAG_H
#include <QString>

namespace CommanderSpellbookBracketTag
{
enum class BracketTag
{
    Ruthless,
    Spicy,
    Powerful,
    Oddball,
    PreconAppropriate,
    Casual,
    Unknown
};

inline static BracketTag bracketTagFromString(const QString &s)
{
    if (s == "R")
        return BracketTag::Ruthless;
    if (s == "S")
        return BracketTag::Spicy;
    if (s == "P")
        return BracketTag::Powerful;
    if (s == "O")
        return BracketTag::Oddball;
    if (s == "PA")
        return BracketTag::PreconAppropriate;
    if (s == "C")
        return BracketTag::Casual;
    return BracketTag::Unknown;
}

inline static QString bracketTagToString(BracketTag tag)
{
    switch (tag) {
        case BracketTag::Ruthless:
            return "Ruthless";
        case BracketTag::Spicy:
            return "Spicy";
        case BracketTag::Powerful:
            return "Powerful";
        case BracketTag::Oddball:
            return "Oddball";
        case BracketTag::PreconAppropriate:
            return "Precon Appropriate";
        case BracketTag::Casual:
            return "Casual";
        case BracketTag::Unknown:
            return "Unknown";
    }
    return {};
}

inline static QString bracketTagToOfficialString(BracketTag tag)
{
    switch (tag) {
        case BracketTag::Ruthless:
            return "[5] cEDH";
        case BracketTag::Spicy:
            return "[4] Optimized";
        case BracketTag::Powerful:
            return "[3] Upgraded";
        case BracketTag::Oddball:
            return "[2] Core";
        case BracketTag::PreconAppropriate:
            return "[1] Exhibition";
        case BracketTag::Casual:
            return "[1] Casual";
        case BracketTag::Unknown:
            return "Unknown";
    }
    return {};
}

} // namespace CommanderSpellbookBracketTag

#endif // COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_TAG_H
