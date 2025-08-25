#ifndef COCKATRICE_GAME_EVENT_HANDLER_H
#define COCKATRICE_GAME_EVENT_HANDLER_H

#include "pb/event_leave.pb.h"
#include "pb/serverinfo_player.pb.h"
#include "player/player.h"

#include <QObject>

class AbstractClient;
class TabGame;
class Response;
class GameEventContainer;
class GameEventContext;
class GameCommand;
class GameState;
class MessageLogWidget;
class CommandContainer;
class Event_GameJoined;
class Event_GameStateChanged;
class Event_PlayerPropertiesChanged;
class Event_Join;
class Event_Leave;
class Event_GameHostChanged;
class Event_GameClosed;
class Event_GameStart;
class Event_SetActivePlayer;
class Event_SetActivePhase;
class Event_Ping;
class Event_GameSay;
class Event_Kicked;
class Event_ReverseTurn;
class PendingCommand;

class GameEventHandler : public QObject
{
    Q_OBJECT

private:
    TabGame *game;
    GameState *gameState;
    MessageLogWidget *messageLog;

public:
    GameEventHandler(TabGame *game, MessageLogWidget *messageLog);

    void handleNextTurn();
    void handleReverseTurn();

    void handlePlayerConceded();
    void handlePlayerUnconceded();
    void handleActivePhaseChanged(int phase);
    void handleGameLeft();
    void handleChatMessageSent(const QString &chatMessage);
    void handleArrowDeletion(int arrowId);

    void eventSpectatorSay(const Event_GameSay &event, int eventPlayerId, const GameEventContext &context);
    void eventSpectatorLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);

    void eventGameStateChanged(const Event_GameStateChanged &event, int eventPlayerId, const GameEventContext &context);
    void processCardAttachmentsForPlayers(const Event_GameStateChanged &event);
    void eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event,
                                      int eventPlayerId,
                                      const GameEventContext &context);
    void eventJoin(const Event_Join &event, int eventPlayerId, const GameEventContext &context);
    void eventLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);
    QString getLeaveReason(Event_Leave::LeaveReason reason);
    void eventKicked(const Event_Kicked &event, int eventPlayerId, const GameEventContext &context);
    void eventGameHostChanged(const Event_GameHostChanged &event, int eventPlayerId, const GameEventContext &context);
    void eventGameClosed(const Event_GameClosed &event, int eventPlayerId, const GameEventContext &context);

    void eventSetActivePlayer(const Event_SetActivePlayer &event, int eventPlayerId, const GameEventContext &context);
    void eventSetActivePhase(const Event_SetActivePhase &event, int eventPlayerId, const GameEventContext &context);
    void eventPing(const Event_Ping &event, int eventPlayerId, const GameEventContext &context);
    void eventReverseTurn(const Event_ReverseTurn &event, int eventPlayerId, const GameEventContext & /*context*/);

    void commandFinished(const Response &response);

    void processGameEventContainer(const GameEventContainer &cont,
                                   AbstractClient *client,
                                   Player::EventProcessingOptions options);
    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);
public slots:
    void sendGameCommand(PendingCommand *pend, int playerId = -1);
    void sendGameCommand(const ::google::protobuf::Message &command, int playerId = -1);

signals:
    void localPlayerDeckSelected(Player *localPlayer, int playerId, ServerInfo_Player playerInfo);
    void remotePlayerDeckSelected(QString deckList, int playerId, QString playerName);
    void remotePlayersDecksSelected(QVector<QPair<int, QPair<QString, QString>>> opponentDecks);
    void localPlayerSideboardLocked(int playerId, bool sideboardLocked);
    void localPlayerReadyStateChanged(int playerId, bool ready);
    void gameStopped();
    void gameClosed();
    void playerLeft(Player *leavingPlayer);
    void playerKicked();
};

#endif // COCKATRICE_GAME_EVENT_HANDLER_H
