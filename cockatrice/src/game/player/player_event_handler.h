/**
 * @file player_event_handler.h
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_EVENT_HANDLER_H
#define COCKATRICE_PLAYER_EVENT_HANDLER_H
#include "event_processing_options.h"

#include <QObject>
#include <libcockatrice/protocol/pb/game_event.pb.h>
#include <libcockatrice/protocol/pb/game_event_context.pb.h>

class CardItem;
class CardZoneLogic;
class Player;
class Event_AttachCard;
class Event_ChangeZoneProperties;
class Event_CreateArrow;
class Event_CreateCounter;
class Event_CreateToken;
class Event_DelCounter;
class Event_DeleteArrow;
class Event_DestroyCard;
class Event_DrawCards;
class Event_DumpZone;
class Event_FlipCard;
class Event_GameSay;
class Event_MoveCard;
class Event_RevealCards;
class Event_RollDie;
class Event_SetCardAttr;
class Event_SetCardCounter;
class Event_SetCounter;
class Event_Shuffle;
class PlayerEventHandler : public QObject
{

    Q_OBJECT
signals:
    void logSay(Player *player, QString message);
    void logShuffle(Player *player, CardZoneLogic *zone, int start, int end);
    void logRollDie(Player *player, int sides, const QList<uint> &rolls);
    void logCreateArrow(Player *player,
                        Player *startPlayer,
                        QString startCard,
                        Player *targetPlayer,
                        QString targetCard,
                        bool _playerTarget);
    void logCreateToken(Player *player, QString cardName, QString pt, bool faceDown);
    void logDrawCards(Player *player, int number, bool deckIsEmpty);
    void logUndoDraw(Player *player, QString cardName);
    void logMoveCard(Player *player,
                     CardItem *card,
                     CardZoneLogic *startZone,
                     int oldX,
                     CardZoneLogic *targetZone,
                     int newX);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logDestroyCard(Player *player, QString cardName);
    void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
    void logUnattachCard(Player *player, QString cardName);
    void logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue);
    void logSetTapped(Player *player, CardItem *card, bool tapped);
    void logSetCounter(Player *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap);
    void logSetPT(Player *player, CardItem *card, QString newPT);
    void logSetAnnotation(Player *player, CardItem *card, QString newAnnotation);
    void logDumpZone(Player *player, CardZoneLogic *zone, int numberCards, bool isReversed = false);
    void logRevealCards(Player *player,
                        CardZoneLogic *zone,
                        int cardId,
                        QString cardName,
                        Player *otherPlayer,
                        bool faceDown,
                        int amount,
                        bool isLentToAnotherPlayer = false);
    void logAlwaysRevealTopCard(Player *player, CardZoneLogic *zone, bool reveal);
    void logAlwaysLookAtTopCard(Player *player, CardZoneLogic *zone, bool reveal);

public:
    PlayerEventHandler(Player *player);

    void processGameEvent(GameEvent::GameEventType type,
                          const GameEvent &event,
                          const GameEventContext &context,
                          EventProcessingOptions options);

    void eventGameSay(const Event_GameSay &event);
    void eventShuffle(const Event_Shuffle &event);
    void eventRollDie(const Event_RollDie &event);
    void eventCreateArrow(const Event_CreateArrow &event);
    void eventDeleteArrow(const Event_DeleteArrow &event);
    void eventCreateToken(const Event_CreateToken &event);
    void
    eventSetCardAttr(const Event_SetCardAttr &event, const GameEventContext &context, EventProcessingOptions options);
    void eventSetCardCounter(const Event_SetCardCounter &event);
    void eventCreateCounter(const Event_CreateCounter &event);
    void eventSetCounter(const Event_SetCounter &event);
    void eventDelCounter(const Event_DelCounter &event);
    void eventDumpZone(const Event_DumpZone &event);
    void eventMoveCard(const Event_MoveCard &event, const GameEventContext &context);
    void eventFlipCard(const Event_FlipCard &event);
    void eventDestroyCard(const Event_DestroyCard &event);
    void eventAttachCard(const Event_AttachCard &event);
    void eventDrawCards(const Event_DrawCards &event);
    void eventRevealCards(const Event_RevealCards &event, EventProcessingOptions options);
    void eventChangeZoneProperties(const Event_ChangeZoneProperties &event);

private:
    Player *player;
};

#endif // COCKATRICE_PLAYER_EVENT_HANDLER_H
