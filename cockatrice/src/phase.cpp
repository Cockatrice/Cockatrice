#include "phase.h"


Phase Phases::getPhase(int phase)
{
    return phases[phase];
}

const Phase Phases::phases[Phases::phaseTypesCount] = {{"Untap", "green", "untap_step"},
                                                       {"Upkeep", "green", "upkeep_step"},
                                                       {"Draw", "green", "draw_step"},
                                                       {"First Main", "blue", "main_1"},
                                                       {"Beginning of Combat", "red", "start_combat"},
                                                       {"Declare Attackers", "red", "attack_step"},
                                                       {"Declare Blockers", "red", "block_step"},
                                                       {"Combat Damage", "red", "damage_step"},
                                                       {"End of Combat", "red", "end_combat"},
                                                       {"Second Main", "blue", "main_2"},
                                                       {"End/Cleanup", "green", "end_step"},
                                                       {"Unknown Phase", "black", "unkwown"}};
