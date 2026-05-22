/**
 * @file player_event_handler.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_PLAYER_EVENT_HANDLER_H
#define COCKATRICE_PLAYER_EVENT_HANDLER_H
#include "event_processing_options.h"

#include <QObject>
#include <libcockatrice/protocol/pb/card_attributes.pb.h>
#include <libcockatrice/protocol/pb/game_event.pb.h>
#include <libcockatrice/protocol/pb/game_event_context.pb.h>

class CardItem;
class CardZoneLogic;
class PlayerLogic;
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
class Event_SetCounterActive;
class Event_Shuffle;
class Event_GameLogNotice;

class PlayerEventHandler : public QObject
{

    Q_OBJECT
signals:
    void logSay(PlayerLogic *player, QString message);
    void logShuffle(PlayerLogic *player, CardZoneLogic *zone, int start, int end);
    void logRollDie(PlayerLogic *player, int sides, const QList<uint> &rolls);
    void logCreateArrow(PlayerLogic *player,
                        PlayerLogic *startPlayer,
                        QString startCard,
                        PlayerLogic *targetPlayer,
                        QString targetCard,
                        bool _playerTarget);
    void logCreateToken(PlayerLogic *player, QString cardName, QString pt, bool faceDown);
    void logDrawCards(PlayerLogic *player, int number, bool deckIsEmpty);
    void logUndoDraw(PlayerLogic *player, QString cardName);
    void logUndoDrawFailed(PlayerLogic *player);
    void logMoveCard(PlayerLogic *player,
                     CardItem *card,
                     CardZoneLogic *startZone,
                     int oldX,
                     CardZoneLogic *targetZone,
                     int newX);
    void logFlipCard(PlayerLogic *player, QString cardName, bool faceDown);
    void logDestroyCard(PlayerLogic *player, QString cardName);
    void logAttachCard(PlayerLogic *player, QString cardName, PlayerLogic *targetPlayer, QString targetCardName);
    void logUnattachCard(PlayerLogic *player, QString cardName);
    void logSetCardCounter(PlayerLogic *player, QString cardName, int counterId, int value, int oldValue);
    void logSetTapped(PlayerLogic *player, CardItem *card, bool tapped);
    void logSetCounter(PlayerLogic *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(PlayerLogic *player, CardItem *card, bool doesntUntap);
    void logSetPT(PlayerLogic *player, CardItem *card, QString newPT);
    void logSetAnnotation(PlayerLogic *player, CardItem *card, QString newAnnotation);
    void logDumpZone(PlayerLogic *player, CardZoneLogic *zone, int numberCards, bool isReversed = false);
    void logRevealCards(PlayerLogic *player,
                        CardZoneLogic *zone,
                        int cardId,
                        QString cardName,
                        PlayerLogic *otherPlayer,
                        bool faceDown,
                        int amount,
                        bool isLentToAnotherPlayer = false);
    void logAlwaysRevealTopCard(PlayerLogic *player, CardZoneLogic *zone, bool reveal);
    void logAlwaysLookAtTopCard(PlayerLogic *player, CardZoneLogic *zone, bool reveal);
    void cardZoneChanged(CardItem *card, bool sameZone);
    void requestCardMenuUpdate(const CardItem *card);

public:
    PlayerEventHandler(PlayerLogic *player);

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
    void eventSetCounterActive(const Event_SetCounterActive &event);
    void eventDelCounter(const Event_DelCounter &event);
    void eventDumpZone(const Event_DumpZone &event);
    void eventMoveCard(const Event_MoveCard &event, const GameEventContext &context);
    void eventFlipCard(const Event_FlipCard &event);
    void eventDestroyCard(const Event_DestroyCard &event);
    void eventAttachCard(const Event_AttachCard &event);
    void eventDrawCards(const Event_DrawCards &event);
    void eventRevealCards(const Event_RevealCards &event, EventProcessingOptions options);
    void eventChangeZoneProperties(const Event_ChangeZoneProperties &event);
    void eventGameLogNotice(const Event_GameLogNotice &event);

private:
    PlayerLogic *player;

    void setCardAttrHelper(const GameEventContext &context,
                           CardItem *card,
                           CardAttribute attribute,
                           const QString &avalue,
                           bool allCards,
                           EventProcessingOptions options);
};

#endif // COCKATRICE_PLAYER_EVENT_HANDLER_H
