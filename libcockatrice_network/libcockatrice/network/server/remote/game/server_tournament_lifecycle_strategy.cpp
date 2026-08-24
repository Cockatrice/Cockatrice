#include "server_tournament_lifecycle_strategy.h"

#include "server_abstract_player.h"
#include "server_game.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(TournamentLifecycleLog, "tournament_lifecycle");

Server_GameLifecycleStrategy::StartAction Server_TournamentLifecycleStrategy::onGameStarting(Server_Game *game)
{
    // Match sub-games start through the normal flow; only the tournament hub game is
    // managed by this lifecycle.
    if (game->getTournamentParentGame() != nullptr) {
        return StartAction::ProceedNormal;
    }

    for (auto *player : game->getPlayers().values()) {
        if (!player->getDeckList()) {
            qCWarning(TournamentLifecycleLog)
                << "Tournament cannot start: player" << player->getUserInfo()->name().c_str() << "has no deck";
            return StartAction::Handled;
        }
    }

    if (!game->getIsTournamentGame()) {
        qCWarning(TournamentLifecycleLog) << "Tournament lifecycle used for non-tournament game — falling back";
        return StartAction::ProceedNormal;
    }

    game->startTournament();
    return StartAction::Handled;
}
