#ifndef ABSTRACT_PARTICIPANT_H
#define ABSTRACT_PARTICIPANT_H

#include "../serverinfo_user_container.h"
#include "server_arrowtarget.h"

#include <QMutex>
#include <libcockatrice/protocol/pb/card_attributes.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>

class Server_Game;
class Server_AbstractUserInterface;
class ServerInfo_User;
class ServerInfo_Player;
class ServerInfo_PlayerProperties;
class GameEventContainer;
class GameEventStorage;
class ResponseContainer;
class GameCommand;

class Command_KickFromGame;
class Command_LeaveGame;
class Command_GameSay;
class Command_Shuffle;
class Command_Mulligan;
class Command_RollDie;
class Command_DrawCards;
class Command_UndoDraw;
class Command_FlipCard;
class Command_AttachCard;
class Command_CreateToken;
class Command_CreateArrow;
class Command_DeleteArrow;
class Command_SetCardAttr;
class Command_SetCardCounter;
class Command_IncCardCounter;
class Command_ReadyStart;
class Command_Concede;
class Command_Unconcede;
class Command_Judge;
class Command_IncCounter;
class Command_CreateCounter;
class Command_SetCounter;
class Command_DelCounter;
class Command_NextTurn;
class Command_SetActivePhase;
class Command_DumpZone;
class Command_RevealCards;
class Command_ReverseTurn;
class Command_MoveCard;
class Command_SetSideboardPlan;
class Command_DeckSelect;
class Command_SetSideboardLock;
class Command_ChangeZoneProperties;

class Server_AbstractParticipant : public Server_ArrowTarget, public ServerInfo_User_Container
{
    Q_OBJECT
protected:
    Server_Game *game;
    Server_AbstractUserInterface *userInterface;
    int pingTime;
    int playerId;
    bool spectator;
    bool judge;
    virtual void getPlayerProperties(ServerInfo_PlayerProperties &result);
    mutable QMutex playerMutex;

public:
    Server_AbstractParticipant(Server_Game *_game,
                               int _playerId,
                               const ServerInfo_User &_userInfo,
                               bool _judge,
                               Server_AbstractUserInterface *_handler);
    ~Server_AbstractParticipant() override;
    virtual void prepareDestroy()
    {
        removeFromGame();
    };
    void removeFromGame();
    Server_AbstractUserInterface *getUserInterface() const
    {
        return userInterface;
    }
    void setUserInterface(Server_AbstractUserInterface *_userInterface);
    void disconnectClient();

    int getPlayerId() const
    {
        return playerId;
    }
    bool getSpectator() const
    {
        return spectator;
    }
    bool getJudge() const
    {
        return judge;
    }
    Server_Game *getGame() const
    {
        return game;
    }
    int getPingTime() const
    {
        return pingTime;
    }
    bool updatePingTime();
    void getProperties(ServerInfo_PlayerProperties &result, bool withUserInfo);

    virtual Response::ResponseCode
    cmdLeaveGame(const Command_LeaveGame &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdKickFromGame(const Command_KickFromGame &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode cmdConcede(const Command_Concede &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdUnconcede(const Command_Unconcede &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode cmdJudge(const Command_Judge &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdDeckSelect(const Command_DeckSelect &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdSetSideboardLock(const Command_SetSideboardLock &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode cmdGameSay(const Command_GameSay &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode cmdShuffle(const Command_Shuffle &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdMulligan(const Command_Mulligan &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdRollDie(const Command_RollDie &cmd, ResponseContainer &rc, GameEventStorage &ges) const;
    virtual Response::ResponseCode
    cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdUndoDraw(const Command_UndoDraw &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdCreateArrow(const Command_CreateArrow &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdDeleteArrow(const Command_DeleteArrow &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdSetCardCounter(const Command_SetCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdIncCardCounter(const Command_IncCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdIncCounter(const Command_IncCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdCreateCounter(const Command_CreateCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdSetCounter(const Command_SetCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdDelCounter(const Command_DelCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdNextTurn(const Command_NextTurn &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdSetActivePhase(const Command_SetActivePhase &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer &rc, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdReverseTurn(const Command_ReverseTurn & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges);
    virtual Response::ResponseCode
    cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd, ResponseContainer &rc, GameEventStorage &ges);

    Response::ResponseCode processGameCommand(const GameCommand &command, ResponseContainer &rc, GameEventStorage &ges);
    void sendGameEvent(const GameEventContainer &event);

    virtual void
    getInfo(ServerInfo_Player *info, Server_AbstractParticipant *recipient, bool omniscient, bool withUserInfo);
};

#endif
