#include "abstract_game.h"

#include "player/player.h"

AbstractGame::AbstractGame(TabGame *_tab) : tab(_tab)
{
    gameMetaInfo = new GameMetaInfo(this);
    gameEventHandler = new GameEventHandler(this);

    activeCard = nullptr;
}

bool AbstractGame::isHost() const
{
    return gameState->getHostId() == playerManager->getLocalPlayerId();
}

AbstractClient *AbstractGame::getClientForPlayer(int playerId) const
{
    if (gameState->getClients().size() > 1) {
        if (playerId == -1) {
            playerId = playerManager->getActiveLocalPlayer(gameState->getActivePlayer())->getPlayerInfo()->getId();
        }

        return gameState->getClients().at(playerId);
    } else if (gameState->getClients().isEmpty())
        return nullptr;
    else
        return gameState->getClients().first();
}

void AbstractGame::loadReplay(GameReplay *replay)
{
    gameMetaInfo->setFromProto(replay->game_info());
    gameMetaInfo->setSpectatorsOmniscient(true);
}

void AbstractGame::setActiveCard(CardItem *card)
{
    activeCard = card;
}

CardItem *AbstractGame::getCard(int playerId, const QString &zoneName, int cardId) const
{
    Player *player = playerManager->getPlayer(playerId);
    if (!player)
        return nullptr;

    CardZoneLogic *zone = player->getZones().value(zoneName, 0);
    if (!zone)
        return nullptr;

    return zone->getCard(cardId);
}