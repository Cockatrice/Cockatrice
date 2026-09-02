#ifndef SERVER_TOURNAMENT_H
#define SERVER_TOURNAMENT_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QRecursiveMutex>
#include <QSet>
#include <libcockatrice/protocol/pb/event_tournament_state.pb.h>

class DeckList;
class Server_Game;
class Server_MatchGameFactory;
class Server_AbstractParticipant;
class Server_AbstractUserInterface;
class GameEventStorage;

/** @brief Maximum number of games per match a tournament can be configured with. */
constexpr int MAX_GAMES_PER_MATCH = 5;

class Server_Tournament : public QObject
{
    Q_OBJECT
public:
    explicit Server_Tournament(Server_Game *_parentGame, Server_MatchGameFactory *_factory, QObject *parent = nullptr);
    ~Server_Tournament() override;

    void addPlayer(int playerId, const QString &playerName);
    void removePlayer(int playerId);
    // Marks an already-starting/started tournament player as dropped: they stop
    // being paired and their outstanding unstarted match is awarded as a loss.
    void dropPlayer(int playerId);
    void startTournament();
    void advanceRound(GameEventStorage &ges);
    void recordMatchResult(int playerId1, int playerId2, int winnerId, GameEventStorage &ges);
    bool recordMatchResultByGameId(int gameId, int winnerId, GameEventStorage &ges);
    void broadcastTournamentState(GameEventStorage &ges);

    bool isStarted() const
    {
        return started;
    }
    bool isAllDecksSubmitted() const;
    int getPlayerCount() const
    {
        return players.size();
    }
    int getTournamentPlayerIdByName(const QString &name) const;
    void setPlayerDeckSubmitted(int playerId)
    {
        if (players.contains(playerId)) {
            players[playerId].deckSubmitted = true;
        }
    }
    void setPlayerDeck(int playerId, DeckList *deck);

    void setGamesPerMatch(uint32_t n)
    {
        gamesPerMatch = n;
    }
    uint32_t getGamesPerMatch() const
    {
        return gamesPerMatch;
    }

    struct TournamentPlayerData
    {
        int playerId;
        QString playerName;
        int wins = 0;
        int losses = 0;
        int draws = 0;
        bool deckSubmitted = false;
        bool dropped = false;
    };

    struct TournamentPairingData
    {
        int player1Id;
        int player2Id;
        int gameId = -1;
        int winnerId = -2; // -2 = undecided, -1 = draw, >= 0 = winner player id
        int player1MatchWins = 0;
        int player2MatchWins = 0;
        QList<int> matchGameIds;
    };

private:
    QPointer<Server_Game> parentGame;
    Server_MatchGameFactory *matchGameFactory;
    mutable QRecursiveMutex tournamentMutex;
    QMap<int, TournamentPlayerData> players;
    QMap<int, DeckList *> submittedDecks;
    QList<TournamentPairingData> currentPairings;
    // Players that have already received a bye in a previous round, so no one
    // gets more than one bye over the whole tournament.
    QSet<int> byeGivenPlayers;
    QList<QPair<int, int>> allPreviousPairings;
    int currentRound;
    int totalRounds;
    bool started;
    uint32_t gamesPerMatch = 1;

    void generateSwissPairings();
    int calculateTotalRounds() const;
    bool havePlayed(int p1, int p2) const;
    bool allPairingsDecided() const;
    void createMatchGame(int player1Id, int player2Id);
    void enqueueMatchGameCreation();
    void checkAndAdvanceRound(GameEventStorage &ges);
};

#endif // SERVER_TOURNAMENT_H
