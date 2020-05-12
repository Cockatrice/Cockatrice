#include "phase.h"

Phase::Phase(const char *_name, QString _color, QString _soundFileName) : color(_color), soundFileName(_soundFileName)
{
    name = tr(_name);
}

Phase Phases::getPhase(int phase)
{
    if (0 <= phase && phase < Phases::phaseTypesCount) {
        return phases[phase];
    } else {
        return unknownPhase;
    }
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
