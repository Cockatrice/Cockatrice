#include "commander_spellbook_bracket_explainer.h"

static QString cardList(const QVector<CommanderSpellbookCardResult> &cards, int max = 5)
{
    QStringList names;
    for (int i = 0; i < cards.size() && i < max; ++i) {
        names << cards[i].name;
    }

    if (cards.size() > max) {
        names << QString("and %1 more").arg(cards.size() - max);
    }

    return names.join(", ");
}

static QString comboCount(const QVector<CommanderSpellbookVariantResult> &variants)
{
    return QString::number(variants.size());
}

BracketExplanation BracketExplainer::explain(const EstimateBracketResult &r)
{
    BracketExplanation out;
    out.bracket = r.bracketTag;

    if (!r.gameChangerCards.isEmpty()) {
        BracketExplanationSection s;
        s.title = "Game-changing cards";

        s.bulletPoints << QString("Your deck contains %1 game-changing cards, such as %2.")
                              .arg(r.gameChangerCards.size())
                              .arg(cardList(r.gameChangerCards));

        out.sections << s;
    }

    if (!r.extraTurnCards.isEmpty() || !r.extraTurnTemplates.isEmpty() || !r.extraTurnCombos.isEmpty()) {

        BracketExplanationSection s;
        s.title = "Extra turns";

        if (!r.extraTurnCards.isEmpty()) {
            s.bulletPoints << QString("The deck contains %1 extra-turn cards (%2).")
                                  .arg(r.extraTurnCards.size())
                                  .arg(cardList(r.extraTurnCards));
        }

        if (!r.extraTurnTemplates.isEmpty()) {
            s.bulletPoints << QString("%1 extra-turn templates were identified.").arg(comboCount(r.extraTurnTemplates));
        }

        if (!r.extraTurnCombos.isEmpty()) {
            s.bulletPoints
                << QString("%1 extra-turn combo variants were identified.").arg(comboCount(r.extraTurnCombos));
        }

        out.sections << s;
    }

    if (!r.massLandDenialCards.isEmpty() || !r.massLandDenialTemplates.isEmpty() || !r.massLandDenialCombos.isEmpty()) {

        BracketExplanationSection s;
        s.title = "Mass land denial";

        if (!r.massLandDenialCards.isEmpty()) {
            s.bulletPoints << QString("The deck contains %1 mass land denial cards (%2).")
                                  .arg(r.massLandDenialCards.size())
                                  .arg(cardList(r.massLandDenialCards));
        }

        if (!r.massLandDenialTemplates.isEmpty()) {
            s.bulletPoints
                << QString("%1 mass land denial templates were identified.").arg(comboCount(r.massLandDenialTemplates));
        }

        if (!r.massLandDenialCombos.isEmpty()) {
            s.bulletPoints << QString("%1 mass land denial combo variants were identified.")
                                  .arg(comboCount(r.massLandDenialCombos));
        }

        out.sections << s;
    }

    if (!r.lockCombos.isEmpty() || !r.skipTurnsCombos.isEmpty()) {

        BracketExplanationSection s;
        s.title = "Lock pieces";

        if (!r.lockCombos.isEmpty()) {
            s.bulletPoints << QString("%1 lock combo variants were detected.").arg(comboCount(r.lockCombos));
        }

        if (!r.skipTurnsCombos.isEmpty()) {
            s.bulletPoints << QString("%1 skip-turn combo variants were detected.").arg(comboCount(r.skipTurnsCombos));
        }

        out.sections << s;
    }

    if (!r.definitelyTwoCardCombos.isEmpty() || !r.arguablyTwoCardCombos.isEmpty()) {

        BracketExplanationSection s;
        s.title = "Two-card combos";

        if (!r.definitelyTwoCardCombos.isEmpty()) {
            s.bulletPoints << QString("%1 definite two-card combo variants were identified.")
                                  .arg(comboCount(r.definitelyTwoCardCombos));
        }

        if (!r.arguablyTwoCardCombos.isEmpty()) {
            s.bulletPoints << QString("%1 arguable two-card combo variants were identified.")
                                  .arg(comboCount(r.arguablyTwoCardCombos));
        }

        out.sections << s;
    }

    return out;
}
