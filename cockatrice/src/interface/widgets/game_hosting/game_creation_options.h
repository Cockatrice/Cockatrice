/**
 * @file game_creation_options.h
 * @ingroup RoomWidgets
 */

#ifndef GAME_CREATION_OPTIONS_H
#define GAME_CREATION_OPTIONS_H

#include "libcockatrice/protocol/pb/room_commands.pb.h"

#include <QList>
#include <QMap>
#include <QString>

class IGameSettingsProvider;
class ServerInfo_Game;

/**
 * @brief The values needed to create a game, independent of the widgets that edit them.
 *
 * Pure value object: it holds no widget or settings dependencies, so every host entry
 * point can build a @ref Command_CreateGame through the same conversion.
 */
struct GameCreationOptions
{
    QString description;
    QString password;
    int maxPlayers = 2;
    bool onlyBuddies = false;
    bool onlyRegistered = false;
    bool spectatorsAllowed = true;
    bool spectatorsNeedPassword = false;
    bool spectatorsCanTalk = false;
    bool spectatorsSeeEverything = false;
    bool joinAsJudge = false;
    bool joinAsSpectator = false;
    int startingLifeTotal = 20;
    bool shareDecklistsOnLoad = false;
    bool shufflePlayers = false;
    QList<int> gameTypeIds;

    /**
     * @brief Reads the remembered create-game settings.
     *
     * Join restrictions are only meaningful for registered users, so they are left
     * unset for an unregistered host. Game type ids are resolved from the room's map
     * by matching the remembered names.
     */
    [[nodiscard]] static GameCreationOptions
    fromSettings(const IGameSettingsProvider &settings, const QMap<int, QString> &gameTypes, bool registeredUser);

    /**
     * @brief Reads the read-only fields shown in the game information view.
     */
    [[nodiscard]] static GameCreationOptions fromGameInfo(const ServerInfo_Game &gameInfo,
                                                          const QMap<int, QString> &gameTypes);

    /**
     * @brief Builds the room command, simplifying the description's whitespace.
     */
    [[nodiscard]] Command_CreateGame toCreateGameCommand() const;
};

#endif
