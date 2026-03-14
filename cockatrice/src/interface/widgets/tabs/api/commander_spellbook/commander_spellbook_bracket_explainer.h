#ifndef COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_EXPLAINER_H
#define COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_EXPLAINER_H
#include "api_response/commander_spellbook_estimate_bracket_result.h"

struct BracketExplanationSection
{
    QString title;
    QStringList bulletPoints;
};

struct BracketExplanation
{
    CommanderSpellbookBracketTag::BracketTag bracket;
    QList<BracketExplanationSection> sections;

    bool isEmpty() const
    {
        return sections.isEmpty();
    }
};

class BracketExplainer
{
public:
    static BracketExplanation explain(const EstimateBracketResult &result);
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_EXPLAINER_H
