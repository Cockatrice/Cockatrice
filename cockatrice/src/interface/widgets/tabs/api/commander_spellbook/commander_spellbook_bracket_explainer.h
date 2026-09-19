#ifndef COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_EXPLAINER_H
#define COCKATRICE_COMMANDER_SPELLBOOK_BRACKET_EXPLAINER_H
#include "api_response/commander_spellbook_estimate_bracket_result.h"

namespace CommanderBracketNames
{
inline const char *CommanderSpellbookBracketNames = QT_TR_NOOP("CommanderSpellbook");
inline const char *OfficialCommanderBracketNames = QT_TR_NOOP("Official (approximate)");
inline const char *Explainer = QT_TR_NOOP(
    "The bracket system combines both objective data, as well as subjective play experience to estimate a "
    "bracket for a deck.\nCommanderSpellbook's estimation is algorithmical, which means that it can only operate "
    "on the objective data, not the subjective intent. \nThey have chosen to represent this by defining their "
    "own bracket system which matches their algorithm.\n"
    "This custom bracket system maps loosely to the standard system. \nYou may choose to use these mapped "
    "standardized names if these are more familiar to you, however, you should keep in mind that these are just "
    "rough estimations.\n\nAlways consider the subjective factors of the bracket system when determing a deck's "
    "final bracket!");
} // namespace CommanderBracketNames

struct BracketExplanationSection
{
    QString title;
    QStringList bulletPoints;
};

struct BracketExplanation
{
    QString bracket;
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
