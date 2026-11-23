#include "game_creation_options.h"

#include <QSet>
#include <libcockatrice/interfaces/interface_game_settings_provider.h>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

GameCreationOptions GameCreationOptions::fromSettings(const IGameSettingsProvider &settings,
                                                      const QMap<int, QString> &gameTypes,
                                                      bool registeredUser)
{
    GameCreationOptions options;
    options.description = settings.getGameDescription();
    options.maxPlayers = settings.getMaxPlayers();
    if (registeredUser) {
        options.onlyBuddies = settings.getOnlyBuddies();
        options.onlyRegistered = settings.getOnlyRegistered();
    }
    options.spectatorsAllowed = settings.getSpectatorsAllowed();
    options.spectatorsNeedPassword = settings.getSpectatorsNeedPassword();
    options.spectatorsCanTalk = settings.getSpectatorsCanTalk();
    options.spectatorsSeeEverything = settings.getSpectatorsCanSeeEverything();
    options.joinAsSpectator = settings.getCreateGameAsSpectator();
    options.startingLifeTotal = settings.getDefaultStartingLifeTotal();
    options.shareDecklistsOnLoad = settings.getShareDecklistsOnLoad();
    options.shufflePlayers = settings.getShufflePlayers();

    // The old dialog stored game type names as "Name, " in one string; keep the exact
    // substring test so an existing game.ini selects the same types after the split.
    const QString savedGameTypes = settings.getGameTypes();
    for (auto it = gameTypes.cbegin(); it != gameTypes.cend(); ++it) {
        if (savedGameTypes.contains(it.value() + ", ")) {
            options.gameTypeIds.append(it.key());
        }
    }

    return options;
}

GameCreationOptions GameCreationOptions::defaultsFor(bool registeredUser)
{
    GameCreationOptions options;
    options.onlyRegistered = registeredUser;
    return options;
}

GameCreationOptions GameCreationOptions::fromGameInfo(const ServerInfo_Game &gameInfo,
                                                      const QMap<int, QString> &gameTypes)
{
    GameCreationOptions options;
    options.description = QString::fromStdString(gameInfo.description());
    options.maxPlayers = gameInfo.max_players();
    options.onlyBuddies = gameInfo.only_buddies();
    options.onlyRegistered = gameInfo.only_registered();
    options.spectatorsAllowed = gameInfo.spectators_allowed();
    options.spectatorsNeedPassword = gameInfo.spectators_need_password();
    options.spectatorsCanTalk = gameInfo.spectators_can_chat();
    options.spectatorsSeeEverything = gameInfo.spectators_omniscient();
    options.shareDecklistsOnLoad = gameInfo.share_decklists_on_load();
    options.shufflePlayers = gameInfo.shuffle_players();

    QSet<int> selectedTypes;
    for (int i = 0; i < gameInfo.game_types_size(); ++i) {
        selectedTypes.insert(gameInfo.game_types(i));
    }
    for (auto it = gameTypes.cbegin(); it != gameTypes.cend(); ++it) {
        if (selectedTypes.contains(it.key())) {
            options.gameTypeIds.append(it.key());
        }
    }

    return options;
}

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
