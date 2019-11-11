#include "phase.h"


QString Phase::getName(){
    return names[index];
}

QString Phase::getColor(){
    return colors[index];
}

QString Phase::getSoundFileName(){
    return soundFileNames[index];
}

const QString Phase::names[12] = {"Untap", "Upkeep", "Draw", "First Main", "Beginning of Combat", "Declare Attackers",
                "Declare Blockers", "Combat Damage", "End of Combat", "Second Main", "End/Cleanup"};
const QString Phase::colors[12] = {"green", "green", "green", "blue", "red", "red", "red", "red", "red", "blue", "green", "black"};
const QString Phase::soundFileNames[12] = {"untap_step", "upkeep_step", "draw_step", "main_1", "start_combat", "attack_step",
                         "block_step", "damage_step", "end_combat", "main_2", "end_step", "unkwown"};
