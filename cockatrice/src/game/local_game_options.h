/**
 * @file local_game_options.h
 * @ingroup Game
 * @brief Configuration options for local games.
 *
 * This struct encapsulates all settings that can be configured when starting
 * a local game, providing a clean interface that prevents parameter explosion
 * as more options are added in the future.
 *
 * @note User documentation: doc/doxygen/extra-pages/user_documentation/local_game/starting_a_local_game.md
 */

#ifndef LOCAL_GAME_OPTIONS_H
#define LOCAL_GAME_OPTIONS_H

struct LocalGameOptions
{
    int numberPlayers = 1;
    int startingLifeTotal = 20;
    bool spectatorsSeeEverything = false;
};

#endif // LOCAL_GAME_OPTIONS_H
