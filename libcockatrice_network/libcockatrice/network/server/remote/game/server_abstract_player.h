#ifndef ABSTRACT_PLAYER_H
#define ABSTRACT_PLAYER_H

#include "../serverinfo_user_container.h"
#include "server_abstract_participant.h"

#include <QMap>
#include <QString>

class CardToMove;
class DeckList;
class Server_Arrow;
class Server_Card;
class Server_CardZone;
class Server_Counter;
struct MoveCardStruct;

class Server_AbstractPlayer : public Server_AbstractParticipant
{
    Q_OBJECT
private:
    class MoveCardCompareFunctor;
    QMap<int, Server_Arrow *> arrows;

    void sendCreateTokenEvents(Server_CardZone *zone, Server_Card *card, int xCoord, int yCoord, GameEventStorage &ges);
    void getPlayerProperties(ServerInfo_PlayerProperties &result) override;

protected:
    bool conceded;
    DeckList *deck;
    bool sideboardLocked;
    QMap<QString, Server_CardZone *> zones;
    bool readyStart;
    int nextCardId;

    void revealTopCardIfNeeded(Server_CardZone *zone, GameEventStorage &ges);

public:
    Server_AbstractPlayer(Server_Game *_game,
                          int _playerId,
                          const ServerInfo_User &_userInfo,
                          bool _judge,
                          Server_AbstractUserInterface *_handler);
    ~Server_AbstractPlayer() override;
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
    const QMap<int, Server_Arrow *> &getArrows() const
    {
        return arrows;
    }

    int newCardId();
    int newArrowId() const;

    void addZone(Server_CardZone *zone);
    void addArrow(Server_Arrow *arrow);
    void updateArrowId(int id);
    bool deleteArrow(int arrowId);

    virtual void setupZones();
    virtual void clearZones();

    Response::ResponseCode moveCard(GameEventStorage &ges,
                                    Server_CardZone *startzone,
                                    const QList<const CardToMove *> &_cards,
                                    Server_CardZone *targetzone,
                                    int xCoord,
                                    int yCoord,
                                    bool fixFreeSpaces = true,
                                    bool undoingDraw = false,
                                    bool isReversed = false);
    virtual void onCardBeingMoved(GameEventStorage &ges,
                                  const MoveCardStruct &cardStruct,
                                  Server_CardZone *startzone,
                                  Server_CardZone *targetzone,
                                  bool undoingDraw);

    void unattachCard(GameEventStorage &ges, Server_Card *card);
    Response::ResponseCode setCardAttrHelper(GameEventStorage &ges,
                                             int targetPlayerId,
                                             const QString &zone,
                                             int cardId,
                                             CardAttribute attribute,
                                             const QString &attrValue,
                                             Server_Card *unzonedCard = nullptr);

    virtual Response::ResponseCode
    cmdConcede(const Command_Concede &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdUnconcede(const Command_Unconcede &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdRollDie(const Command_RollDie &cmd, ResponseContainer &rc, GameEventStorage &ges) const override;
    virtual Response::ResponseCode
    cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdCreateArrow(const Command_CreateArrow &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdDeleteArrow(const Command_DeleteArrow &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdSetCardCounter(const Command_SetCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdIncCardCounter(const Command_IncCardCounter &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode
    cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer &rc, GameEventStorage &ges) override;
    virtual Response::ResponseCode cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                           ResponseContainer &rc,
                                                           GameEventStorage &ges) override;

    virtual void getInfo(ServerInfo_Player *info,
                         Server_AbstractParticipant *playerWhosAsking,
                         bool omniscient,
                         bool withUserInfo) override;
};

#endif
