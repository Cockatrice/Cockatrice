#ifndef COCKATRICE_ARCHIDEKT_FORMATS_H
#define COCKATRICE_ARCHIDEKT_FORMATS_H
#include <QHash>
#include <QString>

namespace ArchidektFormats
{
enum class DeckFormat
{
    Standard = 0,
    Modern = 1,
    Commander = 2,
    Legacy = 3,
    Vintage = 4,
    Pauper = 5,
    Custom = 6,
    Frontier = 7,
    FutureStandard = 8,
    PennyDreadful = 9,
    Commander1v1 = 10,
    DualCommander = 11,
    Brawl = 12,

    // Values outside Archidekt range
    Alchemy = 1000,
    Historic = 1001,
    Gladiator = 1002,
    Oathbreaker = 1003,
    OldSchool = 1004,
    PauperCommander = 1005,
    Pioneer = 1006,
    PreDH = 1007,
    Premodern = 1008,
    StandardBrawl = 1009,
    Timeless = 1010,
    Unknown = 1011
};

inline static QString formatToApiName(DeckFormat format)
{
    switch (format) {
        case DeckFormat::Standard:
            return "Standard";
        case DeckFormat::Modern:
            return "Modern";
        case DeckFormat::Commander:
            return "Commander";
        case DeckFormat::Legacy:
            return "Legacy";
        case DeckFormat::Vintage:
            return "Vintage";
        case DeckFormat::Pauper:
            return "Pauper";
        case DeckFormat::Custom:
            return "Custom";
        case DeckFormat::Frontier:
            return "Frontier";
        case DeckFormat::FutureStandard:
            return "Future Std";
        case DeckFormat::PennyDreadful:
            return "Penny Dreadful";
        case DeckFormat::Commander1v1:
            return "1v1 Commander";
        case DeckFormat::DualCommander:
            return "Dual Commander";
        case DeckFormat::Brawl:
            return "Brawl";

        case DeckFormat::Alchemy:
            return "Alchemy";
        case DeckFormat::Historic:
            return "Historic";
        case DeckFormat::Gladiator:
            return "Gladiator";
        case DeckFormat::Oathbreaker:
            return "Oathbreaker";
        case DeckFormat::OldSchool:
            return "Old School";
        case DeckFormat::PauperCommander:
            return "Pauper Commander";
        case DeckFormat::Pioneer:
            return "Pioneer";
        case DeckFormat::PreDH:
            return "PreDH";
        case DeckFormat::Premodern:
            return "Premodern";
        case DeckFormat::StandardBrawl:
            return "Standard Brawl";
        case DeckFormat::Timeless:
            return "Timeless";

        default:
            return "Unknown";
    }
}

inline static DeckFormat apiNameToFormat(const QString &name)
{
    const QString n = name.trimmed();

    if (n.compare("Standard", Qt::CaseInsensitive) == 0)
        return DeckFormat::Standard;
    if (n.compare("Modern", Qt::CaseInsensitive) == 0)
        return DeckFormat::Modern;
    if (n.compare("Commander", Qt::CaseInsensitive) == 0)
        return DeckFormat::Commander;
    if (n.compare("Legacy", Qt::CaseInsensitive) == 0)
        return DeckFormat::Legacy;
    if (n.compare("Vintage", Qt::CaseInsensitive) == 0)
        return DeckFormat::Vintage;
    if (n.compare("Pauper", Qt::CaseInsensitive) == 0)
        return DeckFormat::Pauper;
    if (n.compare("Custom", Qt::CaseInsensitive) == 0)
        return DeckFormat::Custom;
    if (n.compare("Frontier", Qt::CaseInsensitive) == 0)
        return DeckFormat::Frontier;
    if (n.compare("Future Std", Qt::CaseInsensitive) == 0)
        return DeckFormat::FutureStandard;
    if (n.compare("Penny Dreadful", Qt::CaseInsensitive) == 0)
        return DeckFormat::PennyDreadful;
    if (n.compare("1v1 Commander", Qt::CaseInsensitive) == 0)
        return DeckFormat::Commander1v1;
    if (n.compare("Dual Commander", Qt::CaseInsensitive) == 0)
        return DeckFormat::DualCommander;
    if (n.compare("Brawl", Qt::CaseInsensitive) == 0)
        return DeckFormat::Brawl;

    if (n.compare("Alchemy", Qt::CaseInsensitive) == 0)
        return DeckFormat::Alchemy;
    if (n.compare("Historic", Qt::CaseInsensitive) == 0)
        return DeckFormat::Historic;
    if (n.compare("Gladiator", Qt::CaseInsensitive) == 0)
        return DeckFormat::Gladiator;
    if (n.compare("Oathbreaker", Qt::CaseInsensitive) == 0)
        return DeckFormat::Oathbreaker;
    if (n.compare("Old School", Qt::CaseInsensitive) == 0)
        return DeckFormat::OldSchool;
    if (n.compare("Pauper Commander", Qt::CaseInsensitive) == 0)
        return DeckFormat::PauperCommander;
    if (n.compare("Pioneer", Qt::CaseInsensitive) == 0)
        return DeckFormat::Pioneer;
    if (n.compare("PreDH", Qt::CaseInsensitive) == 0)
        return DeckFormat::PreDH;
    if (n.compare("Premodern", Qt::CaseInsensitive) == 0)
        return DeckFormat::Premodern;
    if (n.compare("Standard Brawl", Qt::CaseInsensitive) == 0)
        return DeckFormat::StandardBrawl;
    if (n.compare("Timeless", Qt::CaseInsensitive) == 0)
        return DeckFormat::Timeless;

    return DeckFormat::Unknown;
}

inline static QString formatToCockatriceName(DeckFormat format)
{
    switch (format) {
        case DeckFormat::Standard:
            return "standard";
        case DeckFormat::Modern:
            return "modern";
        case DeckFormat::Commander:
            return "commander";
        case DeckFormat::Legacy:
            return "legacy";
        case DeckFormat::Vintage:
            return "vintage";
        case DeckFormat::Pauper:
            return "pauper";
        case DeckFormat::Brawl:
            return "brawl";
        case DeckFormat::PennyDreadful:
            return "penny";
        case DeckFormat::FutureStandard:
            return "future";
        case DeckFormat::Commander1v1:
            return "duel";
        case DeckFormat::DualCommander:
            return "duel";
        case DeckFormat::Alchemy:
            return "alchemy";
        case DeckFormat::Historic:
            return "historic";
        case DeckFormat::Gladiator:
            return "gladiator";
        case DeckFormat::Oathbreaker:
            return "oathbreaker";
        case DeckFormat::OldSchool:
            return "oldschool";
        case DeckFormat::PauperCommander:
            return "paupercommander";
        case DeckFormat::Pioneer:
            return "pioneer";
        case DeckFormat::PreDH:
            return "predh";
        case DeckFormat::Premodern:
            return "premodern";
        case DeckFormat::StandardBrawl:
            return "standardbrawl";
        case DeckFormat::Timeless:
            return "timeless";

        default:
            return {};
    }
}

inline static DeckFormat cockatriceNameToFormat(const QString &apiName)
{
    static const QHash<QString, DeckFormat> map = {
        {"standard", DeckFormat::Standard},     {"modern", DeckFormat::Modern},
        {"commander", DeckFormat::Commander},   {"legacy", DeckFormat::Legacy},
        {"vintage", DeckFormat::Vintage},       {"pauper", DeckFormat::Pauper},
        {"brawl", DeckFormat::Brawl},           {"penny", DeckFormat::PennyDreadful},
        {"future", DeckFormat::FutureStandard}, {"duel", DeckFormat::Commander1v1},
        {"alchemy", DeckFormat::Alchemy},       {"historic", DeckFormat::Historic},
        {"gladiator", DeckFormat::Gladiator},   {"oathbreaker", DeckFormat::Oathbreaker},
        {"oldschool", DeckFormat::OldSchool},   {"paupercommander", DeckFormat::PauperCommander},
        {"pioneer", DeckFormat::Pioneer},       {"predh", DeckFormat::PreDH},
        {"premodern", DeckFormat::Premodern},   {"standardbrawl", DeckFormat::StandardBrawl},
        {"timeless", DeckFormat::Timeless}};

    return map.value(apiName.toLower(), DeckFormat::Unknown);
}

} // namespace ArchidektFormats

#endif // COCKATRICE_ARCHIDEKT_FORMATS_H
