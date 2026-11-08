#include "game_creation_options.h"

Command_CreateGame GameCreationOptions::toCreateGameCommand() const
{
    Command_CreateGame cmd;
    cmd.set_description(description.simplified().toStdString());
    cmd.set_password(password.toStdString());
    cmd.set_max_players(maxPlayers);
    cmd.set_only_buddies(onlyBuddies);
    cmd.set_only_registered(onlyRegistered);
    cmd.set_spectators_allowed(spectatorsAllowed);
    cmd.set_spectators_need_password(spectatorsNeedPassword);
    cmd.set_spectators_can_talk(spectatorsCanTalk);
    cmd.set_spectators_see_everything(spectatorsSeeEverything);
    cmd.set_join_as_judge(joinAsJudge);
    cmd.set_join_as_spectator(joinAsSpectator);
    cmd.set_starting_life_total(startingLifeTotal);
    cmd.set_share_decklists_on_load(shareDecklistsOnLoad);
    cmd.set_shuffle_players(shufflePlayers);
    for (int gameTypeId : gameTypeIds) {
        cmd.add_game_type_ids(gameTypeId);
    }
    return cmd;
}
