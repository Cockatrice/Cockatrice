#include "commander_spellbook_estimate_bracket_result.h"

static void parseCards(const QJsonObject &json, const QString &key, QVector<CommanderSpellbookCardResult> &out)
{
    out.clear();
    for (const auto &v : json.value(key).toArray()) {
        if (!v.isObject())
            continue;
        CommanderSpellbookCardResult c;
        c.fromJson(v.toObject());
        out.append(c);
    }
}

static void parseVariants(const QJsonObject &json, const QString &key, QVector<CommanderSpellbookVariantResult> &out)
{
    out.clear();
    for (const auto &v : json.value(key).toArray()) {
        if (!v.isObject())
            continue;
        CommanderSpellbookVariantResult vr;
        vr.fromJson(v.toObject());
        out.append(vr);
    }
}

void EstimateBracketResult::fromJson(const QJsonObject &json)
{
    bracketTag = CommanderSpellbookBracketTag::bracketTagFromString(json.value("bracketTag").toString());

    parseCards(json, "gameChangerCards", gameChangerCards);
    parseCards(json, "massLandDenialCards", massLandDenialCards);
    parseCards(json, "extraTurnCards", extraTurnCards);
    parseCards(json, "tutorCards", tutorCards);

    parseVariants(json, "massLandDenialTemplates", massLandDenialTemplates);
    parseVariants(json, "massLandDenialCombos", massLandDenialCombos);
    parseVariants(json, "extraTurnTemplates", extraTurnTemplates);
    parseVariants(json, "extraTurnsCombos", extraTurnsCombos);
    parseVariants(json, "tutorTemplates", tutorTemplates);
    parseVariants(json, "lockCombos", lockCombos);
    parseVariants(json, "skipTurnsCombos", skipTurnsCombos);
    parseVariants(json, "definitelyEarlyGameTwoCardCombos", definitelyEarlyGameTwoCardCombos);
    parseVariants(json, "arguablyEarlyGameTwoCardCombos", arguablyEarlyGameTwoCardCombos);
    parseVariants(json, "definitelyLateGameTwoCardCombos", definitelyLateGameTwoCardCombos);
    parseVariants(json, "borderlineLateGameTwoCardCombos", borderlineLateGameTwoCardCombos);
}
