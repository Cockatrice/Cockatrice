#ifndef COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H
#define COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H

#include "commander_spellbook_card_result.h"
#include "commander_spellbook_variant_result.h"

#include <QVector>

class EstimateBracketResult
{
public:
    void fromJson(const QJsonObject &json);

    QString bracketTag;

    QVector<CommanderSpellbookCardResult> gameChangerCards;
    QVector<CommanderSpellbookCardResult> massLandDenialCards;
    QVector<CommanderSpellbookCardResult> extraTurnCards;

    QVector<CommanderSpellbookVariantResult> massLandDenialTemplates;
    QVector<CommanderSpellbookVariantResult> extraTurnTemplates;

    QVector<CommanderSpellbookVariantResult> massLandDenialCombos;
    QVector<CommanderSpellbookVariantResult> extraTurnCombos;
    QVector<CommanderSpellbookVariantResult> lockCombos;
    QVector<CommanderSpellbookVariantResult> skipTurnsCombos;

    QVector<CommanderSpellbookVariantResult> definitelyTwoCardCombos;
    QVector<CommanderSpellbookVariantResult> arguablyTwoCardCombos;
};

#endif