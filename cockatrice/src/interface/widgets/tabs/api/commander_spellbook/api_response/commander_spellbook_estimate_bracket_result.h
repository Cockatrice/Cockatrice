#ifndef COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H
#define COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H
#include "commander_spellbook_card_result.h"
#include "commander_spellbook_variant_result.h"

#include <QVector>

class EstimateBracketResult
{
public:
    void fromJson(const QJsonObject &json);

    CommanderSpellbookBracketTag::BracketTag bracketTag = CommanderSpellbookBracketTag::BracketTag::Unknown;

    QVector<CommanderSpellbookCardResult> gameChangerCards;
    QVector<CommanderSpellbookCardResult> massLandDenialCards;
    QVector<CommanderSpellbookCardResult> extraTurnCards;
    QVector<CommanderSpellbookCardResult> tutorCards;

    QVector<CommanderSpellbookVariantResult> massLandDenialTemplates;
    QVector<CommanderSpellbookVariantResult> massLandDenialCombos;
    QVector<CommanderSpellbookVariantResult> extraTurnTemplates;
    QVector<CommanderSpellbookVariantResult> extraTurnsCombos;
    QVector<CommanderSpellbookVariantResult> tutorTemplates;
    QVector<CommanderSpellbookVariantResult> lockCombos;
    QVector<CommanderSpellbookVariantResult> skipTurnsCombos;
    QVector<CommanderSpellbookVariantResult> definitelyEarlyGameTwoCardCombos;
    QVector<CommanderSpellbookVariantResult> arguablyEarlyGameTwoCardCombos;
    QVector<CommanderSpellbookVariantResult> definitelyLateGameTwoCardCombos;
    QVector<CommanderSpellbookVariantResult> borderlineLateGameTwoCardCombos;
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_ESTIMATE_BRACKET_RESULT_H
