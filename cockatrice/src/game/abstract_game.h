/**
 * @file abstract_game.h
 * @ingroup GameLogic
 */
//! \todo Document this file.

#ifndef COCKATRICE_ABSTRACT_GAME_H
#define COCKATRICE_ABSTRACT_GAME_H

#include <QObject>
#include <QString>
#include <qtmetamacros.h>

class CardItem;
class AbstractClient;
class GameEventHandler;
class GameMetaInfo;
class GameReplay;
class GameState;
class PlayerManager;

class AbstractGame : public QObject
{
    Q_OBJECT

public:
    explicit AbstractGame(QObject *parent);

    GameMetaInfo *gameMetaInfo;
    GameState *gameState;
    GameEventHandler *gameEventHandler;
    PlayerManager *playerManager;
    CardItem *activeCard;

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

    CardItem *getCard(int playerId, const QString &zoneName, int cardId) const;

    void setActiveCard(CardItem *card);
    CardItem *getActiveCard() const
    {
        return activeCard;
    }
};

#endif // COCKATRICE_ABSTRACT_GAME_H
