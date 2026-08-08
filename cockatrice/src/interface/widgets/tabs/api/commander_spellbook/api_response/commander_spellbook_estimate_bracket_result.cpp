#include "commander_spellbook_estimate_bracket_result.h"

void EstimateBracketResult::fromJson(const QJsonObject &json)
{
    bracketTag = json.value("bracketTag").toString();

    gameChangerCards.clear();
    massLandDenialCards.clear();
    extraTurnCards.clear();

    massLandDenialTemplates.clear();
    extraTurnTemplates.clear();

    massLandDenialCombos.clear();
    extraTurnCombos.clear();
    lockCombos.clear();
    skipTurnsCombos.clear();

    definitelyTwoCardCombos.clear();
    arguablyTwoCardCombos.clear();

    //
    // Cards
    //
    for (const auto &value : json.value("cards").toArray()) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();

        CommanderSpellbookCardResult card;
        card.fromJson(obj.value("card").toObject());

        if (obj.value("gameChanger").toBool()) {
            gameChangerCards.append(card);
        }

        if (obj.value("massLandDenial").toBool()) {
            massLandDenialCards.append(card);
        }

        if (obj.value("extraTurn").toBool()) {
            extraTurnCards.append(card);
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

        CommanderSpellbookVariantResult variant;
        variant.fromJson(obj);

        if (obj.value("massLandDenial").toBool()) {
            massLandDenialTemplates.append(variant);
        }

        if (obj.value("extraTurn").toBool()) {
            extraTurnTemplates.append(variant);
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

        CommanderSpellbookVariantResult combo;
        combo.fromJson(obj);

        if (obj.value("massLandDenial").toBool()) {
            massLandDenialCombos.append(combo);
        }

        if (obj.value("extraTurn").toBool()) {
            extraTurnCombos.append(combo);
        }

        if (obj.value("lock").toBool()) {
            lockCombos.append(combo);
        }

        if (obj.value("skipTurns").toBool()) {
            skipTurnsCombos.append(combo);
        }

        if (obj.value("definitelyTwoCard").toBool()) {
            definitelyTwoCardCombos.append(combo);
        }

        if (obj.value("arguablyTwoCard").toBool()) {
            arguablyTwoCardCombos.append(combo);
        }
    }
}