#ifndef COCKATRICE_GAME_H
#define COCKATRICE_GAME_H

#include "../../../common/pb/game_replay.pb.h"
#include "game_event_handler.h"
#include "game_meta_info.h"
#include "game_state.h"
#include "player/player_manager.h"

#include <QObject>

class CardItem;
class GameMetaInfo;
class TabGame;
class Game : public QObject
{
    Q_OBJECT

public:
    Game(TabGame *tab,
         QList<AbstractClient *> &_clients,
         const Event_GameJoined &event,
         const QMap<int, QString> &_roomGameTypes);

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

#endif // COCKATRICE_GAME_H
