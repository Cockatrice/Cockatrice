#ifndef PLAYER_H
#define PLAYER_H

#include "server_arrowtarget.h"
#include "serverinfo_user_container.h"
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

#include "pb/card_attributes.pb.h"
#include "pb/response.pb.h"

class DeckList;
class Server_Game;
class Server_CardZone;
class Server_Counter;
class Server_Arrow;
class Server_Card;
class Server_AbstractUserInterface;
class ServerInfo_User;
class ServerInfo_Player;
class ServerInfo_PlayerProperties;
class CommandContainer;
class CardToMove;
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
class Command_IncCounter;
class Command_CreateCounter;
class Command_SetCounter;
class Command_DelCounter;
class Command_NextTurn;
class Command_SetActivePhase;
class Command_DumpZone;
class Command_StopDumpZone;
class Command_RevealCards;
class Command_MoveCard;
class Command_SetSideboardPlan;
class Command_DeckSelect;
class Command_SetSideboardLock;
class Command_ChangeZoneProperties;

class Server_Player : public Server_ArrowTarget, public ServerInfo_User_Container
{
    Q_OBJECT
private:
    class MoveCardCompareFunctor;
    Server_Game *game;
    Server_AbstractUserInterface *userInterface;
    DeckList *deck;
    QMap<QString, Server_CardZone *> zones;
    QMap<int, Server_Counter *> counters;
    QMap<int, Server_Arrow *> arrows;
    QList<int> lastDrawList;
    int pingTime;
    int playerId;
    bool spectator;
    int initialCards;
    int nextCardId;
    bool readyStart;
    bool conceded;
    bool sideboardLocked;

public:
    mutable QMutex playerMutex;
    Server_Player(Server_Game *_game,
                  int _playerId,
                  const ServerInfo_User &_userInfo,
                  bool _spectator,
                  Server_AbstractUserInterface *_handler);
    ~Server_Player() override;
    void prepareDestroy();
    Server_AbstractUserInterface *getUserInterface() const
    {
        return userInterface;
    }
    void setUserInterface(Server_AbstractUserInterface *_userInterface);
    void disconnectClient();

    bool getReadyStart() const
    {
        return readyStart;
    }
    void setReadyStart(bool _readyStart)
    {
        readyStart = _readyStart;
    }
    int getPlayerId() const
    {
        return playerId;
    }
    bool getSpectator() const
    {
        return spectator;
    }
    bool getConceded() const
    {
        return conceded;
    }
    void setConceded(bool _conceded)
    {
        conceded = _conceded;
    }

    Server_Game *getGame() const
    {
        return game;
    }
    const QMap<QString, Server_CardZone *> &getZones() const
    {
        return zones;
    }
    const QMap<int, Server_Counter *> &getCounters() const
    {
        return counters;
    }
    const QMap<int, Server_Arrow *> &getArrows() const
    {
        return arrows;
    }

    int getPingTime() const
    {
        return pingTime;
    }
    void setPingTime(int _pingTime)
    {
        pingTime = _pingTime;
    }
    void getProperties(ServerInfo_PlayerProperties &result, bool withUserInfo);

    int newCardId();
    int newCounterId() const;
    int newArrowId() const;

    void addZone(Server_CardZone *zone);
    void addArrow(Server_Arrow *arrow);
    bool deleteArrow(int arrowId);
    void addCounter(Server_Counter *counter);

    void clearZones();
    void setupZones();

    Response::ResponseCode drawCards(GameEventStorage &ges, int number);
    Response::ResponseCode moveCard(GameEventStorage &ges,
                                    Server_CardZone *startzone,
                                    const QList<const CardToMove *> &_cards,
                                    Server_CardZone *targetzone,
                                    int x,
                                    int y,
                                    bool fixFreeSpaces = true,
                                    bool undoingDraw = false);
    void unattachCard(GameEventStorage &ges, Server_Card *card);
    Response::ResponseCode setCardAttrHelper(GameEventStorage &ges,
                                             int targetPlayerId,
                                             const QString &zone,
                                             int cardId,
                                             CardAttribute attribute,
                                             const QString &attrValue);

    Response::ResponseCode cmdLeaveGame(const Command_LeaveGame &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdKickFromGame(const Command_KickFromGame &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdConcede(const Command_Concede &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdUnconcede(const Command_Unconcede &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdDeckSelect(const Command_DeckSelect &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdSetSideboardLock(const Command_SetSideboardLock &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdGameSay(const Command_GameSay &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdShuffle(const Command_Shuffle &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdMulligan(const Command_Mulligan &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdRollDie(const Command_RollDie &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdUndoDraw(const Command_UndoDraw &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdCreateArrow(const Command_CreateArrow &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdDeleteArrow(const Command_DeleteArrow &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdSetCardCounter(const Command_SetCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdIncCardCounter(const Command_IncCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdIncCounter(const Command_IncCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdCreateCounter(const Command_CreateCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdSetCounter(const Command_SetCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdDelCounter(const Command_DelCounter &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdNextTurn(const Command_NextTurn &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdSetActivePhase(const Command_SetActivePhase &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdStopDumpZone(const Command_StopDumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer &rc, GameEventStorage &ges);
    Response::ResponseCode
    cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd, ResponseContainer &rc, GameEventStorage &ges);

    Response::ResponseCode processGameCommand(const GameCommand &command, ResponseContainer &rc, GameEventStorage &ges);
    void sendGameEvent(const GameEventContainer &event);

    void getInfo(ServerInfo_Player *info, Server_Player *playerWhosAsking, bool omniscient, bool withUserInfo);
};

#endif
