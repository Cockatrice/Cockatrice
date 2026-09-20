#include "commander_spellbook_estimate_bracket_result.h"

EstimateBracketResult EstimateBracketResult::fromJson(const QJsonObject &json)
{
    EstimateBracketResult result;

    result.bracketTag = json.value("bracketTag").toString();

    //
    // Cards
    //
    for (const auto &value : json.value("cards").toArray()) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();

        CommanderSpellbookCardResult card = CommanderSpellbookCardResult::fromJson(obj.value("card").toObject());

        if (obj.value("gameChanger").toBool()) {
            result.gameChangerCards.append(card);
        }

        if (obj.value("massLandDenial").toBool()) {
            result.massLandDenialCards.append(card);
        }

        if (obj.value("extraTurn").toBool()) {
            result.extraTurnCards.append(card);
        }
    }

    //
    // Templates
    //
    for (const auto &value : json.value("templates").toArray()) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();

        CommanderSpellbookVariantResult variant = CommanderSpellbookVariantResult::fromJson(obj);

        if (obj.value("massLandDenial").toBool()) {
            result.massLandDenialTemplates.append(variant);
        }

        if (obj.value("extraTurn").toBool()) {
            result.extraTurnTemplates.append(variant);
        }
    }

    //
    // Combos
    //
    for (const auto &value : json.value("combos").toArray()) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();

        CommanderSpellbookVariantResult combo = CommanderSpellbookVariantResult::fromJson(obj);

        if (obj.value("massLandDenial").toBool()) {
            result.massLandDenialCombos.append(combo);
        }

        if (obj.value("extraTurn").toBool()) {
            result.extraTurnCombos.append(combo);
        }

        if (obj.value("lock").toBool()) {
            result.lockCombos.append(combo);
        }

        if (obj.value("skipTurns").toBool()) {
            result.skipTurnsCombos.append(combo);
        }

        if (obj.value("definitelyTwoCard").toBool()) {
            result.definitelyTwoCardCombos.append(combo);
        }

        if (obj.value("arguablyTwoCard").toBool()) {
            result.arguablyTwoCardCombos.append(combo);
        }
    }

    return result;
}
