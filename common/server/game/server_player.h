#ifndef PLAYER_H
#define PLAYER_H

#include "../../serverinfo_user_container.h"
#include "server_abstract_participant.h"

#include <QList>
#include <QMap>
#include <QString>

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

class Server_Player : public Server_AbstractParticipant
{
    Q_OBJECT
private:
    class MoveCardCompareFunctor;
    DeckList *deck;
    QMap<QString, Server_CardZone *> zones;
    QMap<int, Server_Counter *> counters;
    QMap<int, Server_Arrow *> arrows;
    QList<int> lastDrawList;
    int nextCardId;
    bool readyStart;
    bool conceded;
    bool sideboardLocked;
    void revealTopCardIfNeeded(Server_CardZone *zone, GameEventStorage &ges);
    void sendCreateTokenEvents(Server_CardZone *zone, Server_Card *card, int xCoord, int yCoord, GameEventStorage &ges);
    void getPlayerProperties(ServerInfo_PlayerProperties &result);

public:
    Server_Player(Server_Game *_game,
                  int _playerId,
                  const ServerInfo_User &_userInfo,
                  bool _judge,
                  Server_AbstractUserInterface *_handler);
    ~Server_Player() override;
    void prepareDestroy() override;
    const DeckList *getDeckList() const
    {
        return deck;
    }
    bool getReadyStart() const
    {
        return readyStart;
    }
    void setReadyStart(bool _readyStart)
    {
        readyStart = _readyStart;
    }
    bool getConceded() const
    {
        return conceded;
    }
    void setConceded(bool _conceded)
    {
        conceded = _conceded;
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

    int newCardId();
    int newCounterId() const;
    int newArrowId() const;

    void addZone(Server_CardZone *zone);
    void addArrow(Server_Arrow *arrow);
    void updateArrowId(int id);
    bool deleteArrow(int arrowId);
    void addCounter(Server_Counter *counter);

    void clearZones();
    void setupZones();

    Response::ResponseCode drawCards(GameEventStorage &ges, int number);
    Response::ResponseCode moveCard(GameEventStorage &ges,
                                    Server_CardZone *startzone,
                                    const QList<const CardToMove *> &_cards,
                                    Server_CardZone *targetzone,
                                    int xCoord,
                                    int yCoord,
                                    bool fixFreeSpaces = true,
                                    bool undoingDraw = false,
                                    bool isReversed = false);
    void unattachCard(GameEventStorage &ges, Server_Card *card);
    Response::ResponseCode setCardAttrHelper(GameEventStorage &ges,
                                             int targetPlayerId,
                                             const QString &zone,
                                             int cardId,
                                             CardAttribute attribute,
                                             const QString &attrValue,
                                             Server_Card *unzonedCard = nullptr);

    Response::ResponseCode
    cmdConcede(const Command_Concede &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdUnconcede(const Command_Unconcede &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
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
    cmdRollDie(const Command_RollDie &cmd, ResponseContainer &rc, GameEventStorage &ges) const override;
    Response::ResponseCode
    cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdUndoDraw(const Command_UndoDraw &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdCreateArrow(const Command_CreateArrow &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdDeleteArrow(const Command_DeleteArrow &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdSetCardCounter(const Command_SetCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdIncCardCounter(const Command_IncCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
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
    cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    Response::ResponseCode
    cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
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
