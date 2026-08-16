/**
 * @file game_event_handler.h
 * @ingroup GameLogic
 * @brief Game-level command sender and event dispatcher.
 *
 * GameEventHandler sends commands initiated by the local client to the server
 * and processes incoming game-wide events. It bridges the networking layer
 * (protobuf events received via AbstractClient) with the game model and UI
 * (GameState, PlayerManager, logging, widgets).
 *
 * Player-scoped events are forwarded to PlayerEventHandler instances, while
 * spectator and global game events are handled directly here.
 */

#ifndef COCKATRICE_GAME_EVENT_HANDLER_H
#define COCKATRICE_GAME_EVENT_HANDLER_H

#include "player/event_processing_options.h"

#include <QLoggingCategory>
#include <QObject>
#include <libcockatrice/protocol/pb/event_leave.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>

class AbstractClient;
class AbstractGame;
class CommandContainer;
class GameCommand;
class GameEventContainer;
class GameEventContext;
class PendingCommand;
class PlayerLogic;
class Response;

class Event_GameStateChanged;
class Event_PlayerPropertiesChanged;
class Event_Join;
class Event_Leave;
class Event_GameHostChanged;
class Event_GameClosed;
class Event_SetActivePlayer;
class Event_SetActivePhase;
class Event_GameSay;
class Event_Kicked;
class Event_ReverseTurn;
class Event_Ping;

inline Q_LOGGING_CATEGORY(GameEventHandlerLog, "game_event_handler");

/**
 * @class GameEventHandler
 * @brief Central dispatcher for game-wide commands and events.
 *
 * This class owns no game state itself. Instead, it:
 * - Sends commands to the server on behalf of local players
 * - Receives and dispatches server-side game events
 * - Updates the game model indirectly via Player, GameState, and PlayerManager
 * - Emits high-level signals for UI updates and logging
 */
class GameEventHandler : public QObject
{
    Q_OBJECT

private:
    /** Pointer to the owning game instance. */
    AbstractGame *game;

public:
    /** @name Construction
     *  Lifecycle and ownership.
     *  @{
     */

    /**
     * @brief Construct a GameEventHandler.
     *
     * The handler is owned by the AbstractGame instance and uses it to
     * access the game state, players, and network clients.
     *
     * @param _game Owning game instance (also used as QObject parent).
     */
    explicit GameEventHandler(AbstractGame *_game);

    /** @} */

    /** @name Outgoing game commands
     *  Commands initiated locally and sent to the server.
     *
     * These methods construct and send protobuf commands corresponding
     * to user actions in the UI.
     *  @{
     */

    /** @brief Request advancing the game to the next turn. */
    void handleNextTurn();

    /** @brief Request reversing the current turn order. */
    void handleReverseTurn();

    /** @brief Concede the game for the currently active local player. */
    void handleActiveLocalPlayerConceded();

    /** @brief Undo a previous concede for the active local player. */
    void handleActiveLocalPlayerUnconceded();

    /**
     * @brief Set the active phase of the game.
     *
     * Typically triggered by the active player selecting a new phase.
     *
     * @param phase Phase identifier.
     */
    void handleActivePhaseChanged(int phase);

    /** @brief Leave the current game session. */
    void handleGameLeft();

    /**
     * @brief Send a chat message to all players and spectators.
     *
     * @param chatMessage Message text.
     */
    void handleChatMessageSent(const QString &chatMessage);

    /**
     * @brief Delete an existing arrow.
     *
     * @param arrowId Unique identifier of the arrow to delete.
     */
    void handleArrowDeletion(int creatorId, int arrowId);
    void handleArrowDeletionFinished(const Response &response, int creatorId, int arrowId);

    /** @} */

    /** @name Incoming event processing
     *  Entry points for server-sent events.
     *  @{
     */

    /**
     * @brief Process a container of game events received from the server.
     *
     * This is the main dispatch function for incoming game events.
     * Events are routed to spectator handlers, game-level handlers,
     * or forwarded to PlayerEventHandler instances as appropriate.
     *
     * @param cont Game event container from the server.
     * @param client Client that received the container.
     * @param options Processing flags (e.g. silent, replay).
     */
    void
    processGameEventContainer(const GameEventContainer &cont, AbstractClient *client, EventProcessingOptions options);

    /** @} */

    /** @name Command preparation helpers
     *  Internal helpers for building command containers.
     *  @{
     */

    /**
     * @brief Wrap a single protobuf command in a PendingCommand.
     *
     * @param cmd Protobuf command message.
     * @return Newly allocated PendingCommand (caller takes ownership).
     */
    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);

    /**
     * @brief Wrap multiple protobuf commands in a single PendingCommand.
     *
     * Ownership of the messages in cmdList is transferred to the handler.
     *
     * @param cmdList List of protobuf command messages.
     * @return Newly allocated PendingCommand.
     */
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);

    /** @} */

    /** @name Spectator event handlers
     *  Events originating from spectators.
     *  @{
     */

    /**
     * @brief Handle a spectator chat message.
     */
    void eventSpectatorSay(const Event_GameSay &event, int eventPlayerId, const GameEventContext &context);

    /**
     * @brief Handle a spectator leaving the game.
     */
    void eventSpectatorLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);

    /** @} */

    /** @name Game state event handlers
     *  Events that affect global game state.
     *  @{
     */

    /**
     * @brief Handle a full game state update from the server.
     *
     * Used during game startup, reconnection, and resynchronization.
     */
    void eventGameStateChanged(const Event_GameStateChanged &event, int eventPlayerId, const GameEventContext &context);

    /**
     * @brief Update card attachment relationships for all players.
     *
     * Called after a game state update to ensure attachments are resolved
     * consistently across all zones.
     */
    void processCardAttachmentsForPlayers(const Event_GameStateChanged &event);

    /** @brief Handle a change in game host. */
    void eventGameHostChanged(const Event_GameHostChanged &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle the game being closed by the server. */
    void eventGameClosed(const Event_GameClosed &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle a change of the active player. */
    void eventSetActivePlayer(const Event_SetActivePlayer &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle a change of the active phase. */
    void eventSetActivePhase(const Event_SetActivePhase &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle a turn reversal event. */
    void eventReverseTurn(const Event_ReverseTurn &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle ping / latency updates. */
    void eventPing(const Event_Ping &event, int eventPlayerId, const GameEventContext &context);

    /** @} */

    /** @name Player lifecycle and property handlers
     *  Events related to players joining, leaving, or changing state.
     *  @{
     */

    /**
     * @brief Handle updates to a player's properties.
     *
     * Includes readiness, concede state, deck selection, sideboard lock,
     * and connection state changes.
     */
    void eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event,
                                      int eventPlayerId,
                                      const GameEventContext &context);

    /** @brief Handle a player or spectator joining the game. */
    void eventJoin(const Event_Join &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle a player leaving the game. */
    void eventLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);

    /** @brief Handle the local player being kicked from the game. */
    void eventKicked(const Event_Kicked &event, int eventPlayerId, const GameEventContext &context);

    /**
     * @brief Convert a leave reason enum to a human-readable string.
     */
    QString getLeaveReason(Event_Leave::LeaveReason reason);

    /** @} */

public slots:
    /** @name Command dispatch slots
     *  Low-level command transmission.
     *  @{
     */

    /**
     * @brief Send a prepared PendingCommand.
     *
     * @param pend Pending command to send.
     * @param playerId Player whose client should send the command.
     */
    void sendGameCommand(PendingCommand *pend, int playerId = -1);

    /**
     * @brief Send a single protobuf command.
     *
     * @param command Protobuf command message.
     * @param playerId Player whose client should send the command.
     */
    void sendGameCommand(const ::google::protobuf::Message &command, int playerId = -1);

    /**
     * @brief Called when a PendingCommand finishes execution.
     *
     * Used to detect server-side errors such as chat flood protection.
     */
    void commandFinished(const Response &response);

    /** @} */

signals:
    /** @name Core state signals
     *  @{
     */

    void emitUserEvent();
    void containerProcessingStarted(GameEventContext context);
    void containerProcessingDone();
    void gameFlooded();
    void setContextJudgeName(QString judgeName);

    /** @} */

    /** @name Player and spectator signals
     *  @{
     */

    void addPlayerToAutoCompleteList(QString playerName);
    void localPlayerDeckSelected(PlayerLogic *localPlayer, int playerId, ServerInfo_Player playerInfo);
    void remotePlayerDeckSelected(QString deckList, int playerId, QString playerName);
    void remotePlayersDecksSelected(QVector<QPair<int, QPair<QString, QString>>> opponentDecks);
    void localPlayerSideboardLocked(int playerId, bool sideboardLocked);
    void localPlayerReadyStateChanged(int playerId, bool ready);

    /** @} */

    /** @name Game flow signals
     *  @{
     */

    void gameStopped();
    void gameClosed();
    void playerPropertiesChanged(const ServerInfo_PlayerProperties &prop, int playerId);
    void playerJoined(const ServerInfo_PlayerProperties &playerInfo);
    void playerLeft(int leavingPlayerId);
    void playerKicked();
    void spectatorJoined(const ServerInfo_PlayerProperties &spectatorInfo);
    void spectatorLeft(int leavingSpectatorId);
    void arrowDeleted(int creatorId, int arrowId);

    /** @} */

    /** @name Logging signals
     *  Signals consumed by MessageLogWidget.
     *  @{
     */

    void logSpectatorSay(ServerInfo_User userInfo, QString message);
    void logSpectatorLeave(QString name, QString reason);
    void logGameStart();
    void logReadyStart(PlayerLogic *player);
    void logNotReadyStart(PlayerLogic *player);
    void logDeckSelect(PlayerLogic *player, QString deckHash, int sideboardSize);
    void logSideboardLockSet(PlayerLogic *player, bool sideboardLocked);
    void logConnectionStateChanged(PlayerLogic *player, bool connected);
    void logJoinSpectator(QString spectatorName);
    void logJoinPlayer(PlayerLogic *player);
    void logLeave(PlayerLogic *player, QString reason);
    void logKicked();
    void logTurnReversed(PlayerLogic *player, bool reversed);
    void logGameClosed();
    void logActivePlayer(PlayerLogic *activePlayer);
    void logActivePhaseChanged(int activePhase);
    void logConcede(int playerId);
    void logUnconcede(int playerId);

    /** @} */
};

#endif // COCKATRICE_GAME_EVENT_HANDLER_H
