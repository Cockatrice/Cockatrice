#ifndef PLAYER_H
#define PLAYER_H

#include "server_abstract_player.h"

class Server_Player : public Server_AbstractPlayer
{
    Q_OBJECT
private:
    QMap<int, Server_Counter *> counters;
    QMap<int, int> castCounts; // index (1-5) -> value
    QList<int> lastDrawList;

public:
    Server_Player(Server_Game *_game,
                  int _playerId,
                  const ServerInfo_User &_userInfo,
                  bool _judge,
                  Server_AbstractUserInterface *_handler);
    ~Server_Player() override;
    const QMap<int, Server_Counter *> &getCounters() const
    {
        return counters;
    }
    const QMap<int, int> &getCastCounts() const
    {
        return castCounts;
    }
    int newCounterId() const;
    void addCounter(Server_Counter *counter);

    static Response::ResponseCode
    evaluateDelCounter(bool gameStarted, bool playerConceded, const Server_Counter *counter);

    static Response::ResponseCode
    evaluateModifyCounter(bool gameStarted, bool playerConceded, const Server_Counter *counter);

    static Response::ResponseCode
    evaluateCreateCounter(bool gameStarted, bool playerConceded, const QString &counterName);

    // Cast count validation
    static Response::ResponseCode evaluateCreateCastCount(bool gameStarted,
                                                          bool playerConceded,
                                                          bool commandZoneEnabled,
                                                          int index,
                                                          bool exists,
                                                          bool predecessorExists);

    static Response::ResponseCode evaluateDeleteCastCount(bool gameStarted,
                                                          bool playerConceded,
                                                          bool commandZoneEnabled,
                                                          int index,
                                                          bool exists,
                                                          int value,
                                                          bool successorExists);

    static Response::ResponseCode
    evaluateModifyCastCount(bool gameStarted, bool playerConceded, bool commandZoneEnabled, int index, bool exists);

    void setupZones() override;
    void clearZones() override;

    Response::ResponseCode drawCards(GameEventStorage &ges, int number);
    void onCardBeingMoved(GameEventStorage &ges,
                          const MoveCardStruct &cardStruct,
                          Server_CardZone *startzone,
                          Server_CardZone *targetzone,
                          bool undoingDraw) override;

    Response::ResponseCode
    cmdDeckSelect(const Command_DeckSelect &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetSideboardLock(const Command_SetSideboardLock &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdShuffle(const Command_Shuffle &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdMulligan(const Command_Mulligan &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdUndoDraw(const Command_UndoDraw &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdIncCounter(const Command_IncCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdCreateCounter(const Command_CreateCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetCounter(const Command_SetCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdDelCounter(const Command_DelCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdNextTurn(const Command_NextTurn &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetActivePhase(const Command_SetActivePhase &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdReverseTurn(const Command_ReverseTurn & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges) override;
    Response::ResponseCode cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                   ResponseContainer &rc,
                                                   GameEventStorage &ges) override;

    // Cast count commands
    Response::ResponseCode
    cmdCreateCastCount(const Command_CreateCastCount &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdDeleteCastCount(const Command_DeleteCastCount &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdIncCastCount(const Command_IncCastCount &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetCastCount(const Command_SetCastCount &cmd, ResponseContainer &rc, GameEventStorage &ges) override;

    void getInfo(ServerInfo_Player *info,
                 Server_AbstractParticipant *playerWhosAsking,
                 bool omniscient,
                 bool withUserInfo) override;
};

#endif
