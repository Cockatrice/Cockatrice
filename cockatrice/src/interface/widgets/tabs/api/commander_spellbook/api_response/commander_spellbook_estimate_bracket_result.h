#ifndef COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H
#define COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H

#include "commander_spellbook_card_result.h"
#include "commander_spellbook_variant_result.h"

#include <QJsonObject>
#include <QList>

struct EstimateBracketResult
{
    static EstimateBracketResult fromJson(const QJsonObject &json);

    QString bracketTag;

    QList<CommanderSpellbookCardResult> gameChangerCards;
    QList<CommanderSpellbookCardResult> massLandDenialCards;
    QList<CommanderSpellbookCardResult> extraTurnCards;

    QList<CommanderSpellbookVariantResult> massLandDenialTemplates;
    QList<CommanderSpellbookVariantResult> extraTurnTemplates;

    QList<CommanderSpellbookVariantResult> massLandDenialCombos;
    QList<CommanderSpellbookVariantResult> extraTurnCombos;
    QList<CommanderSpellbookVariantResult> lockCombos;
    QList<CommanderSpellbookVariantResult> skipTurnsCombos;

    QList<CommanderSpellbookVariantResult> definitelyTwoCardCombos;
    QList<CommanderSpellbookVariantResult> arguablyTwoCardCombos;
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H
