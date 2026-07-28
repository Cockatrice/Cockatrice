#ifndef PLAYER_H
#define PLAYER_H

#include "server_abstract_player.h"

class Server_Player : public Server_AbstractPlayer
{
    Q_OBJECT
private:
    QMap<int, Server_Counter *> counters;
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
    int newCounterId() const;
    void addCounter(Server_Counter *counter);

    /** @name Counter command authorization
     *  Decision logic extracted from the corresponding cmd* handlers so it can be unit-tested
     *  in isolation. Each function takes all relevant state as parameters and touches no
     *  instance members, hence static.
     *  @{
     */

    /**
     * @brief Decide whether a client may delete a counter.
     *
     * Reserved tax counters are server-managed and may never be deleted by a client.
     *
     * @param counter Counter with id counterId, or nullptr if the player has no such counter.
     * @return Response::RespOk if permitted, otherwise the error response for the client.
     */
    static Response::ResponseCode
    evaluateDelCounter(bool gameStarted, bool playerConceded, int counterId, const Server_Counter *counter);

    /**
     * @brief Decide whether a client may change a counter's active (visible) state.
     *
     * Only reserved tax counters can be toggled, and one holding a non-zero value must be reset
     * to zero before it can be deactivated.
     *
     * @param counter Counter with id counterId, or nullptr if the player has no such counter.
     * @param requestedActive Active state the client asked for.
     * @return Response::RespOk if permitted, otherwise the error response for the client.
     */
    static Response::ResponseCode evaluateSetCounterActive(bool gameStarted,
                                                           bool playerConceded,
                                                           bool commandZoneEnabled,
                                                           int counterId,
                                                           const Server_Counter *counter,
                                                           bool requestedActive);

    /**
     * @brief Decide whether a client may change a counter's value.
     *
     * Shared by cmdIncCounter and cmdSetCounter. Reserved tax counters may only be modified
     * inside a Commander game and only while active, so an inactive (hidden) tax counter can
     * never accumulate a value behind the scenes.
     *
     * @param counter Counter with id counterId, or nullptr if the player has no such counter.
     * @return Response::RespOk if permitted, otherwise the error response for the client.
     */
    static Response::ResponseCode evaluateModifyCounter(bool gameStarted,
                                                        bool playerConceded,
                                                        bool commandZoneEnabled,
                                                        int counterId,
                                                        const Server_Counter *counter);

    /** @} */

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
    cmdSetCounterActive(const Command_SetCounterActive &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdNextTurn(const Command_NextTurn &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetActivePhase(const Command_SetActivePhase &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdReverseTurn(const Command_ReverseTurn & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges) override;
    Response::ResponseCode cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                   ResponseContainer &rc,
                                                   GameEventStorage &ges) override;

    void getInfo(ServerInfo_Player *info,
                 Server_AbstractParticipant *playerWhosAsking,
                 bool omniscient,
                 bool withUserInfo) override;
};

#endif
