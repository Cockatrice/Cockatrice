/**
 * @file player_event_handler.h
 * @ingroup GameLogicPlayers
 * @brief Player-scoped game event handler.
 *
 * PlayerEventHandler applies game events that affect a single Player’s
 * board state, zones, cards, counters, arrows, and related UI/log output.
 *
 * It is invoked by GameEventHandler after basic routing and validation.
 * Each instance is bound 1:1 to a Player and must never mutate state
 * belonging to other players except where explicitly required by events
 * (e.g. moving cards between players, attaching cards, arrows).
 *
 * This class is intentionally stateful and tightly coupled to Player,
 * PlayerActions, and the board/zones implementation. It performs both
 * model mutation and UI-side bookkeeping (zone views, arrows, menus).
 */

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
class Event_Shuffle;
class Event_GameLogNotice;

/**
 * @class PlayerEventHandler
 * @brief Applies player-specific game events and emits corresponding log signals.
 *
 * Design notes:
 * - All event handlers assume events are authoritative and already validated
 *   by the server.
 * - Most handlers mutate both logical state (CardItem, CardZoneLogic, counters)
 *   and visual/UI state (views, arrows, menus).
 * - Logging signals are emitted *after* or *during* state mutation, depending
 *   on whether later mutations would invalidate log data.
 */
class PlayerEventHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a PlayerEventHandler bound to a Player.
     * @param player Owning player instance.
     */
    explicit PlayerEventHandler(PlayerLogic *player);

    /** @name Event dispatch
     *  @{
     */

    /**
     * @brief Dispatch a generic GameEvent to the appropriate handler.
     *
     * This is the single entry point used by GameEventHandler. It extracts
     * the correct protobuf extension and forwards the event to a typed
     * handler method.
     *
     * @param type Game event type enum.
     * @param event Generic protobuf container.
     * @param context Additional context (undo, judge, etc.).
     * @param options Processing options (UI suppression, reveal behavior).
     */
    void processGameEvent(GameEvent::GameEventType type,
                          const GameEvent &event,
                          const GameEventContext &context,
                          EventProcessingOptions options);

    /** @} */

    /** @name Chat and randomization events
     *  @{
     */

    /// Handle in-game chat messages from this player.
    void eventGameSay(const Event_GameSay &event);

    /// Handle zone shuffle events (typically libraries).
    void eventShuffle(const Event_Shuffle &event);

    /// Handle die roll events.
    void eventRollDie(const Event_RollDie &event);

    /** @} */

    /** @name Arrow and targeting events
     *  @{
     */

    /// Create a visual arrow between cards or players.
    void eventCreateArrow(const Event_CreateArrow &event);

    /// Delete an existing arrow.
    void eventDeleteArrow(const Event_DeleteArrow &event);

    /** @} */

    /** @name Token and card creation
     *  @{
     */

    /// Create a token card in a target zone.
    void eventCreateToken(const Event_CreateToken &event);

    /** @} */

    /** @name Card attribute and counter updates
     *  @{
     */

    /**
     * @brief Set a card attribute (tapped, PT, annotation, etc.).
     *
     * May apply to a single card or all cards in a zone if no card ID
     * is provided by the event.
     */
    void
    eventSetCardAttr(const Event_SetCardAttr &event, const GameEventContext &context, EventProcessingOptions options);

    /// Update a counter attached to a card.
    void eventSetCardCounter(const Event_SetCardCounter &event);

    /// Create a player-level counter.
    void eventCreateCounter(const Event_CreateCounter &event);

    /// Set a player-level counter value.
    void eventSetCounter(const Event_SetCounter &event);

    /// Delete a player-level counter.
    void eventDelCounter(const Event_DelCounter &event);

    /** @} */

    /** @name Zone-level operations
     *  @{
     */

    /// Log a zone dump (e.g. reveal graveyard/library contents).
    void eventDumpZone(const Event_DumpZone &event);

    /**
     * @brief Move a card between zones and/or players.
     *
     * This is one of the most complex handlers:
     * - Removes the card from the start zone
     * - Updates card identity and ownership if needed
     * - Handles attachments and arrows
     * - Emits appropriate move or undo-draw logs
     * - Inserts the card into the target zone
     */
    void eventMoveCard(const Event_MoveCard &event, const GameEventContext &context);

    /// Flip a card face up or face down.
    void eventFlipCard(const Event_FlipCard &event);

    /// Destroy a card and clean up attachments.
    void eventDestroyCard(const Event_DestroyCard &event);

    /// Attach or detach a card to/from another card.
    void eventAttachCard(const Event_AttachCard &event);

    /** @} */

    /** @name Draw and reveal operations
     *  @{
     */

    /// Draw one or more cards from the deck.
    void eventDrawCards(const Event_DrawCards &event);

    /**
     * @brief Reveal cards from a zone.
     *
     * Handles peeking, in-place top-card reveals, full reveal windows,
     * and write-access granting.
     */
    void eventRevealCards(const Event_RevealCards &event, EventProcessingOptions options);

    /** @} */

    /** @name Zone configuration
     *  @{
     */

    /// Update zone visibility and reveal behavior.
    void eventChangeZoneProperties(const Event_ChangeZoneProperties &event);

    /** @} */

    void eventGameLogNotice(const Event_GameLogNotice &event);
signals:
    /** @name Logging signals
     *  @{
     */
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
    /** @} */

    void cardZoneChanged(CardItem *card, bool sameZone);
    void requestCardMenuUpdate(const CardItem *card);

private:
    /** Owning player instance. */
    PlayerLogic *player;

    void setCardAttrHelper(const GameEventContext &context,
                           CardItem *card,
                           CardAttribute attribute,
                           const QString &avalue,
                           bool allCards,
                           EventProcessingOptions options);
};

#endif // COCKATRICE_PLAYER_EVENT_HANDLER_H
