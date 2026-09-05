#include "server_tournament_match_result_strategy.h"

#include "../server_response_containers.h"
#include "server_abstract_player.h"
#include "server_game.h"
#include "server_tournament.h"

#include <libcockatrice/protocol/pb/event_game_closed.pb.h>
#include <libcockatrice/protocol/pb/event_tournament_state.pb.h>

bool Server_TournamentMatchResultStrategy::onGameFinished(Server_Game *game,
                                                          int playing,
                                                          Server_AbstractPlayer *lastPlayer)
{
    // The hub game is owned by the room and may be torn down once its host leaves
    // and no players remain, while the match sub-games keep running. QPointer keeps
    // this link checked so a later-finishing match can't touch freed memory.
    auto *parentGame = game->getTournamentParentGame().data();
    if (!parentGame || !parentGame->getTournament()) {
        return false;
    }

    int winnerId;
    if (playing == 0) {
        winnerId = -1;
    } else {
        QString winnerName = QString::fromStdString(lastPlayer->getUserInfo()->name());
        auto *tournament = parentGame->getTournament();
        winnerId = tournament->getTournamentPlayerIdByName(winnerName);
    }

    GameEventStorage parentGes;
    bool matchDecided = parentGame->getTournament()->recordMatchResultByGameId(game->getGameId(), winnerId, parentGes);
    parentGes.sendToGame(parentGame);

    return matchDecided;
}
