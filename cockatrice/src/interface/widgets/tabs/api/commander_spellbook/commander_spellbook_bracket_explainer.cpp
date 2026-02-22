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

    // --- Game changers ---
    if (!r.gameChangerCards.isEmpty()) {
        BracketExplanationSection s;
        s.title = "Game-changing cards";
        s.bulletPoints << QString("Your deck contains %1 game-changing cards, such as %2.")
                              .arg(r.gameChangerCards.size())
                              .arg(cardList(r.gameChangerCards));
        out.sections << s;
    }

    // --- Tutors ---
    if (!r.tutorCards.isEmpty()) {
        BracketExplanationSection s;
        s.title = "Tutors";
        s.bulletPoints << QString("The deck runs %1 tutor cards, including %2.")
                              .arg(r.tutorCards.size())
                              .arg(cardList(r.tutorCards));
        out.sections << s;
    }

    // --- Extra turns ---
    if (!r.extraTurnCards.isEmpty()) {
        BracketExplanationSection s;
        s.title = "Extra turn effects";
        s.bulletPoints << QString("Extra turn spells were detected (%1), such as %2.")
                              .arg(r.extraTurnCards.size())
                              .arg(cardList(r.extraTurnCards));
        out.sections << s;
    }

    // --- Mass land denial ---
    if (!r.massLandDenialCards.isEmpty() || !r.massLandDenialCombos.isEmpty()) {
        BracketExplanationSection s;
        s.title = "Mass land denial";

        if (!r.massLandDenialCards.isEmpty()) {
            s.bulletPoints << QString("The deck includes %1 mass land denial cards (%2).")
                                  .arg(r.massLandDenialCards.size())
                                  .arg(cardList(r.massLandDenialCards));
        }

        if (!r.massLandDenialCombos.isEmpty()) {
            s.bulletPoints << QString("%1 mass land denial combo variants were identified.")
                                  .arg(comboCount(r.massLandDenialCombos));
        }

        out.sections << s;
    }

    // --- Lock / skip turns ---
    if (!r.lockCombos.isEmpty() || !r.skipTurnsCombos.isEmpty()) {
        BracketExplanationSection s;
        s.title = "Lock or skip-turn combos";

        if (!r.lockCombos.isEmpty()) {
            s.bulletPoints << QString("%1 lock combo variants were detected.").arg(comboCount(r.lockCombos));
        }

        if (!r.skipTurnsCombos.isEmpty()) {
            s.bulletPoints << QString("%1 skip-turn combo variants were detected.").arg(comboCount(r.skipTurnsCombos));
        }

        out.sections << s;
    }

    // --- Early-game combos ---
    if (!r.definitelyEarlyGameTwoCardCombos.isEmpty() || !r.arguablyEarlyGameTwoCardCombos.isEmpty()) {

        BracketExplanationSection s;
        s.title = "Early-game two-card combos";

        if (!r.definitelyEarlyGameTwoCardCombos.isEmpty()) {
            s.bulletPoints << QString("%1 definitely early-game two-card combos were found.")
                                  .arg(comboCount(r.definitelyEarlyGameTwoCardCombos));
        }

        if (!r.arguablyEarlyGameTwoCardCombos.isEmpty()) {
            s.bulletPoints << QString("%1 arguably early-game two-card combos were found.")
                                  .arg(comboCount(r.arguablyEarlyGameTwoCardCombos));
        }

        out.sections << s;
    }

    // --- Late-game combos ---
    if (!r.definitelyLateGameTwoCardCombos.isEmpty() || !r.borderlineLateGameTwoCardCombos.isEmpty()) {

        BracketExplanationSection s;
        s.title = "Late-game two-card combos";

        if (!r.definitelyLateGameTwoCardCombos.isEmpty()) {
            s.bulletPoints << QString("%1 definitely late-game two-card combos were found.")
                                  .arg(comboCount(r.definitelyLateGameTwoCardCombos));
        }

        if (!r.borderlineLateGameTwoCardCombos.isEmpty()) {
            s.bulletPoints << QString("%1 borderline late-game two-card combos were found.")
                                  .arg(comboCount(r.borderlineLateGameTwoCardCombos));
        }

        out.sections << s;
    }

    return out;
}
