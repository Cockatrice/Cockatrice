/**
 * @file abstract_game.h
 * @ingroup GameLogic
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_ABSTRACT_GAME_H
#define COCKATRICE_ABSTRACT_GAME_H

#include "game_event_handler.h"
#include "game_meta_info.h"
#include "game_state.h"
#include "player/player_manager.h"

#include <QObject>
#include <libcockatrice/protocol/pb/game_replay.pb.h>

class CardItem;
class TabGame;
class AbstractGame : public QObject
{
    Q_OBJECT

public:
    explicit AbstractGame(TabGame *tab);

    TabGame *tab;
    GameMetaInfo *gameMetaInfo;
    GameState *gameState;
    GameEventHandler *gameEventHandler;
    PlayerManager *playerManager;
    CardItem *activeCard;

    TabGame *getTab() const
    {
        return tab;
    }

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

    void loadReplay(GameReplay *replay);

    CardItem *getCard(int playerId, const QString &zoneName, int cardId) const;

    void setActiveCard(CardItem *card);
    CardItem *getActiveCard() const
    {
        return activeCard;
    }
};

#endif // COCKATRICE_ABSTRACT_GAME_H
