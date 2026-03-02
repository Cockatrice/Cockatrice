#include "phase.h"

Phase::Phase(const QString &_name, const QString &_color, const QString &_soundFileName)
    : name(_name), color(_color), soundFileName(_soundFileName)
{
}

/**
 * @return The translated name for the phase
 */
QString Phase::getName() const
{
    return tr(name.toUtf8().data());
}

Phase Phases::getPhase(int phase)
{
    if (0 <= phase && phase < Phases::phaseTypesCount) {
        return phases[phase];
    } else {
        return unknownPhase;
    }
}

int Phases::getLastSubphase(int phase)
{
    if (0 <= phase && phase < Phases::phaseTypesCount) {
        return subPhasesEnd[phase];
    } else {
        return phase;
    }
}

QVector<int> getSubPhasesEnd()
{
    QVector<int> array(Phases::phaseTypesCount);
    for (int phaseEnd = Phases::phaseTypesCount - 1; phaseEnd >= 0;) {
        int subPhase = phaseEnd;
        for (; subPhase >= 0 && Phases::phases[phaseEnd].color == Phases::phases[subPhase].color; --subPhase) {
            array[subPhase] = phaseEnd;
        }
        phaseEnd = subPhase;
    }
    return array;
}

const Phase Phases::unknownPhase(QT_TRANSLATE_NOOP("Phase", "Unknown Phase"), "black", "unknown_phase");
const Phase Phases::phases[Phases::phaseTypesCount] = {
    {QT_TRANSLATE_NOOP("Phase", "Untap"), "green", "untap_step"},
    {QT_TRANSLATE_NOOP("Phase", "Upkeep"), "green", "upkeep_step"},
    {QT_TRANSLATE_NOOP("Phase", "Draw"), "green", "draw_step"},
    {QT_TRANSLATE_NOOP("Phase", "First Main"), "blue", "main_1"},
    {QT_TRANSLATE_NOOP("Phase", "Beginning of Combat"), "red", "start_combat"},
    {QT_TRANSLATE_NOOP("Phase", "Declare Attackers"), "red", "attack_step"},
    {QT_TRANSLATE_NOOP("Phase", "Declare Blockers"), "red", "block_step"},
    {QT_TRANSLATE_NOOP("Phase", "Combat Damage"), "red", "damage_step"},
    {QT_TRANSLATE_NOOP("Phase", "End of Combat"), "red", "end_combat"},
    {QT_TRANSLATE_NOOP("Phase", "Second Main"), "blue", "main_2"},
    {QT_TRANSLATE_NOOP("Phase", "End/Cleanup"), "green", "end_step"}};
const QVector<int> Phases::subPhasesEnd = getSubPhasesEnd();
