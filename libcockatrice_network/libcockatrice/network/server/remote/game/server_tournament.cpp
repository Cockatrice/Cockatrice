#include "server_tournament.h"

#include "../server_abstractuserinterface.h"
#include "../server_response_containers.h"
#include "../serverinfo_user_container.h"
#include "game_config.h"
#include "server_abstract_player.h"
#include "server_game.h"
#include "server_match_game_factory.h"
#include "server_player.h"
#include "server_tournament_match_result_strategy.h"

#include <QLoggingCategory>
#include <algorithm>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/pb/event_tournament_state.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>

inline Q_LOGGING_CATEGORY(TournamentLog, "tournament");

Server_Tournament::Server_Tournament(Server_Game *_parentGame, Server_MatchGameFactory *_factory, QObject *parent)
    : QObject(parent), parentGame(_parentGame), matchGameFactory(_factory), currentRound(0), totalRounds(0),
      started(false)
{
}

Server_Tournament::~Server_Tournament()
{
    qDeleteAll(submittedDecks);
}

void Server_Tournament::addPlayer(int playerId, const QString &playerName)
{
    QMutexLocker locker(&tournamentMutex);
    TournamentPlayerData data;
    data.playerId = playerId;
    data.playerName = playerName;
    data.dropped = false;
    players[playerId] = data;
}

void Server_Tournament::setPlayerDeck(int playerId, DeckList *deck)
{
    QMutexLocker locker(&tournamentMutex);
    delete submittedDecks.value(playerId, nullptr);
    submittedDecks[playerId] = deck;
    if (players.contains(playerId)) {
        players[playerId].deckSubmitted = true;
    }
}

void Server_Tournament::removePlayer(int playerId)
{
    QMutexLocker locker(&tournamentMutex);
    players.remove(playerId);
    submittedDecks.remove(playerId);
    byeGivenPlayers.remove(playerId);
}

void Server_Tournament::dropPlayer(int playerId)
{
    QMutexLocker locker(&tournamentMutex);
    if (!players.contains(playerId)) {
        return;
    }
    players[playerId].dropped = true;
    players[playerId].deckSubmitted = false;

    // Any current pairing that involves the dropped player and is not already
    // decided is awarded to the surviving opponent (or recorded as undecided if
    // both dropped). The opponent keeps playing without sitting out a round.
    for (auto &pairing : currentPairings) {
        if (pairing.winnerId != -2) {
            continue;
        }
        bool involvesDropped = (pairing.player1Id == playerId || pairing.player2Id == playerId);
        if (!involvesDropped) {
            continue;
        }
        if (pairing.player1Id == playerId && pairing.player2Id == playerId) {
            continue;
        }
        int opponent = (pairing.player1Id == playerId) ? pairing.player2Id : pairing.player1Id;
        if (players.contains(opponent) && !players[opponent].dropped) {
            pairing.winnerId = opponent;
            players[opponent].wins += 1;
            players[playerId].losses += 1;
        }
        allPreviousPairings.append(qMakePair(pairing.player1Id, pairing.player2Id));
    }
}

void Server_Tournament::startTournament()
{
    {
        QMutexLocker locker(&tournamentMutex);
        if (started) {
            return;
        }
        totalRounds = calculateTotalRounds();
        started = true;
        currentRound = 0;
        generateSwissPairings();
    }

    // Spawn the first round's match games
    enqueueMatchGameCreation();
}

bool Server_Tournament::isAllDecksSubmitted() const
{
    QMutexLocker locker(&tournamentMutex);
    for (auto it = players.constBegin(); it != players.constEnd(); ++it) {
        if (!it->deckSubmitted) {
            return false;
        }
    }
    return true;
}

int Server_Tournament::getTournamentPlayerIdByName(const QString &name) const
{
    QMutexLocker locker(&tournamentMutex);
    for (auto it = players.constBegin(); it != players.constEnd(); ++it) {
        if (it->playerName == name) {
            return it->playerId;
        }
    }
    return -1;
}

void Server_Tournament::generateSwissPairings()
{
    currentPairings.clear();
    QList<int> available;
    for (auto it = players.constBegin(); it != players.constEnd(); ++it) {
        if (!it->dropped) {
            available.append(it->playerId);
        }
    }

    // Sort by wins descending (and by record for tie-breaking)
    std::sort(available.begin(), available.end(), [this](int a, int b) {
        const auto &pa = players[a];
        const auto &pb = players[b];
        if (pa.wins != pb.wins) {
            return pa.wins > pb.wins;
        }
        if (pa.losses != pb.losses) {
            return pa.losses < pb.losses;
        }
        return a < b;
    });

    QSet<int> paired;
    // Try to pair every player, allowing a single rematch only if the greedy pass
    // would otherwise leave any unpaired remainder. Dropped players are never paired.
    int maxRematches = available.size() / 2;
    for (int i = 0; i < available.size(); ++i) {
        if (paired.contains(available[i])) {
            continue;
        }
        for (int j = i + 1; j < available.size(); ++j) {
            if (paired.contains(available[j])) {
                continue;
            }
            bool rematch = havePlayed(available[i], available[j]);
            if (rematch && maxRematches <= 0) {
                continue;
            }
            TournamentPairingData pairing;
            pairing.player1Id = available[i];
            pairing.player2Id = available[j];
            currentPairings.append(pairing);
            paired.insert(available[i]);
            paired.insert(available[j]);
            if (rematch) {
                --maxRematches;
            }
            break;
        }
    }

    // Give a bye to every remaining unpaired eligible player, worst-ranked first.
    // A player receives at most one bye over the whole tournament.
    QList<int> unpaired;
    for (int id : available) {
        if (!paired.contains(id)) {
            unpaired.append(id);
        }
    }
    // Byes go to the lowest-ranked eligible player who has not had one yet.
    std::sort(unpaired.begin(), unpaired.end(), [this](int a, int b) {
        const auto &pa = players[a];
        const auto &pb = players[b];
        if (pa.wins != pb.wins) {
            return pa.wins < pb.wins;
        }
        if (pa.losses != pb.losses) {
            return pa.losses > pb.losses;
        }
        return a > b;
    });

    for (int id : unpaired) {
        if (byeGivenPlayers.contains(id)) {
            // Already used a bye: a dropped opponent or earlier bye means this player
            // simply sits out the round with a free win to keep the bracket moving.
            TournamentPairingData bye;
            bye.player1Id = id;
            bye.player2Id = -1;
            bye.winnerId = id;
            currentPairings.append(bye);
            continue;
        }
        TournamentPairingData bye;
        bye.player1Id = id;
        bye.player2Id = -1;
        bye.winnerId = id;
        currentPairings.append(bye);
        players[id].wins += 1;
        byeGivenPlayers.insert(id);
        allPreviousPairings.append(qMakePair(id, -1));
    }
}

int Server_Tournament::calculateTotalRounds() const
{
    int n = 0;
    for (auto it = players.constBegin(); it != players.constEnd(); ++it) {
        if (!it->dropped) {
            ++n;
        }
    }
    if (n <= 1) {
        return 0;
    }
    // Standard Swiss rounds: ceil(log2(n))
    int rounds = 0;
    while ((1 << rounds) < n) {
        ++rounds;
    }
    return rounds;
}

bool Server_Tournament::havePlayed(int p1, int p2) const
{
    for (const auto &pair : allPreviousPairings) {
        if ((pair.first == p1 && pair.second == p2) || (pair.first == p2 && pair.second == p1)) {
            return true;
        }
    }
    return false;
}

void Server_Tournament::advanceRound(GameEventStorage &ges)
{
    {
        QMutexLocker locker(&tournamentMutex);
        ++currentRound;
        if (currentRound >= totalRounds) {
            broadcastTournamentState(ges);
            return;
        }
        generateSwissPairings();
    }

    enqueueMatchGameCreation();
    broadcastTournamentState(ges);
}

bool Server_Tournament::allPairingsDecided() const
{
    for (const auto &pairing : currentPairings) {
        if (pairing.winnerId == -2) {
            return false;
        }
    }
    return true;
}

void Server_Tournament::enqueueMatchGameCreation()
{
    QList<QPair<int, int>> planned;
    {
        QMutexLocker locker(&tournamentMutex);
        for (const auto &pairing : currentPairings) {
            if (pairing.player2Id == -1 || pairing.winnerId != -2) {
                continue;
            }
            if (players.value(pairing.player1Id).dropped || players.value(pairing.player2Id).dropped) {
                continue;
            }
            if (pairing.matchGameIds.size() >= static_cast<int>(gamesPerMatch)) {
                continue;
            }
            planned.append(qMakePair(pairing.player1Id, pairing.player2Id));
        }
    }
    if (planned.isEmpty()) {
        return;
    }

    // Create the games from the event loop instead of the caller's stack: command
    // processing holds game mutexes, and room registration takes gamesLock, so spawning
    // synchronously would nest lock orders. The queued job runs once this object's
    // owning thread returns to its event loop with no locks held; it is dropped if this
    // tournament is destroyed first.
    QMetaObject::invokeMethod(
        this,
        [this, planned] {
            for (const auto &pair : planned) {
                createMatchGame(pair.first, pair.second);
            }

            GameEventStorage ges;
            broadcastTournamentState(ges);
            ges.sendToGame(parentGame);
        },
        Qt::QueuedConnection);
}

void Server_Tournament::createMatchGame(int player1Id, int player2Id)
{
    if (!matchGameFactory || player2Id == -1) {
        return;
    }

    QString player1Name;
    QString player2Name;
    QString deck1Native;
    QString deck2Native;
    int round = 0;
    int gameNumber = 1;
    {
        QMutexLocker locker(&tournamentMutex);
        player1Name = players.value(player1Id).playerName;
        player2Name = players.value(player2Id).playerName;

        if (submittedDecks.contains(player1Id)) {
            deck1Native = submittedDecks.value(player1Id)->writeToString_Native();
        }
        if (submittedDecks.contains(player2Id)) {
            deck2Native = submittedDecks.value(player2Id)->writeToString_Native();
        }

        round = currentRound;
        for (const auto &pairing : currentPairings) {
            if (pairing.player1Id == player1Id && pairing.player2Id == player2Id) {
                gameNumber = pairing.matchGameIds.size() + 1;
                break;
            }
        }

        // Defense in depth: never exceed the configured series length
        for (const auto &pairing : currentPairings) {
            if (pairing.player1Id == player1Id && pairing.player2Id == player2Id &&
                pairing.matchGameIds.size() >= static_cast<int>(gamesPerMatch)) {
                qCWarning(TournamentLog) << "Refusing to exceed series length for pairing" << player1Id << player2Id;
                return;
            }
        }

        // Bail out if either participant is no longer connected: a match game with
        // zero or one connected player can never finish and would stall the round.
        if (!matchGameFactory->getUserInterface(player1Name) || !matchGameFactory->getUserInterface(player2Name)) {
            qCWarning(TournamentLog) << "Skipping match creation: a player in pairing" << player1Id << player2Id
                                     << "is no longer connected";
            return;
        }
    }

    // Create a sub-game for this match via the factory, copying the real
    // ServerInfo_User so it ships the true user level rather than a fabricated
    // admin identity that would surface in buddy/ignore-list checks.
    ServerInfo_User creatorInfo;
    if (auto *ui = matchGameFactory->getUserInterface(player1Name)) {
        creatorInfo = *ui->getUserInfo();
    } else {
        creatorInfo.set_name(player1Name.toStdString());
    }

    QString gameDesc = gamesPerMatch > 1
                           ? QString("R%1 Match - Game %2 of %3").arg(round).arg(gameNumber).arg(gamesPerMatch)
                           : QString("Tournament Round %1").arg(round);

    GameConfig matchConfig;
    matchConfig.creatorInfo = creatorInfo;
    matchConfig.description = gameDesc;
    matchConfig.maxPlayers = 2;
    matchConfig.startingLifeTotal = parentGame->getStartingLifeTotal();

    int matchGameId = -1;
    auto *matchGame = matchGameFactory->createMatchGame(matchConfig, matchGameId);
    if (!matchGame || matchGameId == -1) {
        return;
    }

    matchGame->setTournamentMatchInfo(parentGame, player1Id, player2Id);
    matchGame->setMatchResultStrategy(new Server_TournamentMatchResultStrategy);
    // A disconnect inside a tournament match must remove the player so the match
    // can be decided; it must not leave them sitting as a half-present participant.
    matchGame->setDisconnectRemovesPlayer(true);
    matchGameFactory->addGameToRoom(matchGame);

    // Store the game ID in the pairing
    {
        QMutexLocker locker(&tournamentMutex);
        for (auto &pairing : currentPairings) {
            if (pairing.player1Id == player1Id && pairing.player2Id == player2Id) {
                pairing.gameId = matchGameId;
                pairing.matchGameIds.append(matchGameId);
                break;
            }
        }
    }

    // Auto-join both players, sending the join event directly through their UIs.
    // Both UI lookups were verified above, so a player can only drop between that
    // check and this add — in which case they get handled by drop processing and
    // the pairing settles on the surviving opponent.
    QMap<int, QPair<Server_AbstractUserInterface *, ResponseContainer *>> joiners;

    auto joinAndSetupPlayer = [&](int pid, const QString &name) {
        Server_AbstractUserInterface *ui = matchGameFactory->getUserInterface(name);
        if (ui) {
            auto *rc = new ResponseContainer(0);
            matchGame->addPlayer(ui, *rc, false, false, false);
            joiners[pid] = qMakePair(ui, rc);
        }
    };

    joinAndSetupPlayer(player1Id, player1Name);
    joinAndSetupPlayer(player2Id, player2Name);

    // Now send the enqueued GameJoined + GameStateChanged events to each player's client.
    for (auto it = joiners.constBegin(); it != joiners.constEnd(); ++it) {
        it.value().first->sendResponseContainer(*it.value().second, Response::RespNothing);
        delete it.value().second;
    }
    joiners.clear();

    // Set decks and mark players as ready in the match game.
    bool anyDeckMissing = false;
    auto matchPlayers = matchGame->getPlayers();
    for (auto *matchPlayer : matchPlayers) {
        const QString name = QString::fromStdString(matchPlayer->getUserInfo()->name());
        QString deckNative;
        if (name == player1Name) {
            deckNative = deck1Native;
        } else if (name == player2Name) {
            deckNative = deck2Native;
        }

        if (!deckNative.isEmpty()) {
            matchPlayer->setDeck(new DeckList(deckNative));
            matchPlayer->setReadyStart(true);
        } else {
            anyDeckMissing = true;
        }
    }

    if (anyDeckMissing) {
        // Not every participant submitted a deck. Do not force-start: that would
        // kick the players without a deck. Leave the match game open so they can
        // select a deck; the host starts it through the normal ready flow.
        return;
    }

    // Start the match game without forcing: both participants are ready and have
    // decks, so there is nothing to kick.
    matchGame->startGameIfReady(false);
}

void Server_Tournament::recordMatchResult(int playerId1, int playerId2, int winnerId, GameEventStorage &ges)
{
    QMutexLocker locker(&tournamentMutex);

    // Find the pairing and set the winner
    for (auto &pairing : currentPairings) {
        if ((pairing.player1Id == playerId1 && pairing.player2Id == playerId2) ||
            (pairing.player1Id == playerId2 && pairing.player2Id == playerId1)) {
            if (pairing.winnerId != -2) {
                return; // Already recorded — defense in depth against double-call
            }
            pairing.winnerId = winnerId;
            break;
        }
    }

    // Update player records
    if (winnerId == -1) {
        // Draw
        players[playerId1].draws += 1;
        players[playerId2].draws += 1;
    } else if (winnerId == playerId1) {
        players[playerId1].wins += 1;
        players[playerId2].losses += 1;
    } else if (winnerId == playerId2) {
        players[playerId2].wins += 1;
        players[playerId1].losses += 1;
    }

    // Store for future pairing avoidance
    allPreviousPairings.append(qMakePair(playerId1, playerId2));

    // Check if all pairings in current round have results
    bool allDecided = true;
    for (const auto &pairing : currentPairings) {
        if (pairing.winnerId == -2) {
            allDecided = false;
            break;
        }
    }

    broadcastTournamentState(ges);

    if (allDecided) {
        advanceRound(ges);
    }
}

bool Server_Tournament::recordMatchResultByGameId(int gameId, int winnerId, GameEventStorage &ges)
{
    bool matchDecided = false;
    bool seriesContinues = false;
    int p1 = -1;
    int p2 = -1;
    {
        QMutexLocker locker(&tournamentMutex);

        // Find the pairing that owns this game
        TournamentPairingData *pairingPtr = nullptr;
        for (auto &pairing : currentPairings) {
            if (pairing.matchGameIds.contains(gameId)) {
                pairingPtr = &pairing;
                break;
            }
        }

        if (!pairingPtr) {
            return false;
        }

        // If the match is already decided, ignore further sub-game results
        if (pairingPtr->winnerId != -2) {
            return true;
        }

        // Increment per-match wins
        if (winnerId == pairingPtr->player1Id) {
            pairingPtr->player1MatchWins += 1;
        } else if (winnerId == pairingPtr->player2Id) {
            pairingPtr->player2MatchWins += 1;
        }
        // Draw (winnerId == -1): counts nothing toward the series but does consume
        // a slot, so a series can still end in a draw when it is exhausted.

        // The winner needs a strict majority of the games in the series.
        const int gamesPlayed = pairingPtr->matchGameIds.size();
        const int gamesNeeded = static_cast<int>(gamesPerMatch / 2 + 1);
        const int gamesRemaining = static_cast<int>(gamesPerMatch) - gamesPlayed;
        matchDecided = (pairingPtr->player1MatchWins >= gamesNeeded) || (pairingPtr->player2MatchWins >= gamesNeeded);
        if (!matchDecided) {
            // Series exhausted without a strict-majority winner (e.g. a drawn Bo3
            // leaves it 1-1): record the match as a draw so the round always advances.
            matchDecided = (gamesRemaining <= 0) && (pairingPtr->player1MatchWins == pairingPtr->player2MatchWins);
        }

        if (matchDecided) {
            // Determine match winner
            int matchWinnerId = -1;
            if (pairingPtr->player1MatchWins >= gamesNeeded) {
                matchWinnerId = pairingPtr->player1Id;
            } else if (pairingPtr->player2MatchWins >= gamesNeeded) {
                matchWinnerId = pairingPtr->player2Id;
            }
            // Otherwise the series was exhausted evenly — matchWinnerId stays -1 (a draw).

            // Set the match winner on the pairing
            pairingPtr->winnerId = matchWinnerId;

            // Update tournament-level player records
            if (matchWinnerId == pairingPtr->player1Id) {
                players[pairingPtr->player1Id].wins += 1;
                players[pairingPtr->player2Id].losses += 1;
            } else if (matchWinnerId == pairingPtr->player2Id) {
                players[pairingPtr->player2Id].wins += 1;
                players[pairingPtr->player1Id].losses += 1;
            } else {
                players[pairingPtr->player1Id].draws += 1;
                players[pairingPtr->player2Id].draws += 1;
            }

            // Store for future pairing avoidance
            allPreviousPairings.append(qMakePair(pairingPtr->player1Id, pairingPtr->player2Id));
        } else {
            // Match not decided — spawn the next sub-game outside all locks
            seriesContinues = true;
            p1 = pairingPtr->player1Id;
            p2 = pairingPtr->player2Id;
        }

        broadcastTournamentState(ges);
    }

    if (seriesContinues) {
        QMetaObject::invokeMethod(
            this,
            [this, p1, p2] {
                createMatchGame(p1, p2);

                GameEventStorage nextGes;
                broadcastTournamentState(nextGes);
                nextGes.sendToGame(parentGame);
            },
            Qt::QueuedConnection);
    }

    checkAndAdvanceRound(ges);

    return matchDecided;
}

void Server_Tournament::checkAndAdvanceRound(GameEventStorage &ges)
{
    bool roundComplete = false;
    {
        QMutexLocker locker(&tournamentMutex);
        roundComplete = allPairingsDecided();
    }

    if (roundComplete) {
        advanceRound(ges);
    }
}

void Server_Tournament::broadcastTournamentState(GameEventStorage &ges)
{
    QMutexLocker locker(&tournamentMutex);

    Event_TournamentState state;

    if (started && currentRound >= totalRounds) {
        state.set_phase(Event_TournamentState::PHASE_FINISHED);
    } else if (started) {
        state.set_phase(Event_TournamentState::PHASE_PLAYING);
    } else {
        state.set_phase(Event_TournamentState::PHASE_DECK_BUILDING);
    }

    state.set_current_round(currentRound);
    state.set_total_rounds(totalRounds);

    // Settings
    TournamentSettings *settings = state.mutable_settings();
    settings->set_games_per_match(gamesPerMatch);

    for (auto it = players.constBegin(); it != players.constEnd(); ++it) {
        TournamentPlayer *p = state.add_players();
        p->set_player_id(it->playerId);
        p->set_player_name(it->playerName.toStdString());
        p->set_wins(it->wins);
        p->set_losses(it->losses);
        p->set_draws(it->draws);
        p->set_deck_submitted(it->deckSubmitted);
    }

    for (const auto &pairing : currentPairings) {
        TournamentPairing *p = state.add_pairings();
        p->set_player1_id(pairing.player1Id);
        p->set_player2_id(pairing.player2Id);
        p->set_game_id(pairing.gameId);
        // -2 = undecided; a decided draw is -1. The is_draw bit distinguishes a
        // reported draw from an unset winner_id on the wire.
        if (pairing.winnerId == -1) {
            p->set_is_draw(true);
        } else if (pairing.winnerId != -2) {
            p->set_winner_id(pairing.winnerId);
        }
        p->set_player1_match_wins(pairing.player1MatchWins);
        p->set_player2_match_wins(pairing.player2MatchWins);
    }

    ges.enqueueGameEvent(state, -1);
}
