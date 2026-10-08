/**
 * @file abstract_game.h
 * @ingroup GameLogic
 */
//! \todo Document this file.

#ifndef COCKATRICE_ABSTRACT_GAME_H
#define COCKATRICE_ABSTRACT_GAME_H

#include "game_event_handler.h"
#include "game_meta_info.h"
#include "game_state.h"
#include "player/player_manager.h"

#include <QObject>
#include <libcockatrice/protocol/pb/game_replay.pb.h>

class CardItem;
class CardState;
class AbstractGame : public QObject
{
    Q_OBJECT

public:
    explicit AbstractGame(QObject *parent);

    GameMetaInfo *gameMetaInfo;
    GameState *gameState;
    GameEventHandler *gameEventHandler;
    PlayerManager *playerManager;

    GameMetaInfo *getGameMetaInfo()
    {
        return gameMetaInfo;
    }

    GameState *getGameState() const
    {
        return gameState;
    }

    GameEventHandler *getGameEventHandler() const
    {
        return gameEventHandler;
    }

    PlayerManager *getPlayerManager() const
    {
        return playerManager;
    }

    bool isHost() const;

    AbstractClient *getClientForPlayer(int playerId) const;

    void loadReplay(const GameReplay *replay);

    CardState *getCard(int playerId, const QString &zoneName, int cardId) const;
};

#endif // COCKATRICE_ABSTRACT_GAME_H
