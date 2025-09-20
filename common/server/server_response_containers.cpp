#include "server_response_containers.h"

#include "game/server_game.h"

#include <google/protobuf/descriptor.h>

GameEventStorageItem::GameEventStorageItem(const ::google::protobuf::Message &_event,
                                           int _playerId,
                                           EventRecipients _recipients)
    : event(new GameEvent), recipients(_recipients)
{
    event->GetReflection()->MutableMessage(event, _event.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(_event);
    event->set_player_id(_playerId);
}

GameEventStorageItem::~GameEventStorageItem()
{
    delete event;
}

GameEventStorage::GameEventStorage() : gameEventContext(0), privatePlayerId(0)
{
}

GameEventStorage::~GameEventStorage()
{
    delete gameEventContext;
    for (int i = 0; i < gameEventList.size(); ++i)
        delete gameEventList[i];
}

void GameEventStorage::setGameEventContext(const ::google::protobuf::Message &_gameEventContext)
{
    delete gameEventContext;
    gameEventContext = new GameEventContext;
    gameEventContext->GetReflection()
        ->MutableMessage(gameEventContext, _gameEventContext.GetDescriptor()->FindExtensionByName("ext"))
        ->CopyFrom(_gameEventContext);
}

void GameEventStorage::enqueueGameEvent(const ::google::protobuf::Message &event,
                                        int playerId,
                                        GameEventStorageItem::EventRecipients recipients,
                                        int _privatePlayerId)
{
    gameEventList.append(new GameEventStorageItem(event, playerId, recipients));
    if (_privatePlayerId != -1)
        privatePlayerId = _privatePlayerId;
}

void GameEventStorage::sendToGame(Server_Game *game)
{
    if (gameEventList.isEmpty())
        return;

    auto *contPrivate = new GameEventContainer;
    auto *contOthers = new GameEventContainer;
    int id = privatePlayerId;
    if (forcedByJudge != -1) {
        contPrivate->set_forced_by_judge(forcedByJudge);
        contOthers->set_forced_by_judge(forcedByJudge);
        if (overwriteOwnership) {
            id = forcedByJudge;
            setOverwriteOwnership(false);
        }
    }

    for (const auto &i : gameEventList) {
        const GameEvent &event = i->getGameEvent();
        const GameEventStorageItem::EventRecipients recipients = i->getRecipients();
        if (recipients.testFlag(GameEventStorageItem::SendToPrivate))
            contPrivate->add_event_list()->CopyFrom(event);
        if (recipients.testFlag(GameEventStorageItem::SendToOthers))
            contOthers->add_event_list()->CopyFrom(event);
    }
    if (gameEventContext) {
        contPrivate->mutable_context()->CopyFrom(*gameEventContext);
        contOthers->mutable_context()->CopyFrom(*gameEventContext);
    }
    game->sendGameEventContainer(contPrivate, GameEventStorageItem::SendToPrivate, id);
    game->sendGameEventContainer(contOthers, GameEventStorageItem::SendToOthers, id);
}

ResponseContainer::ResponseContainer(int _cmdId) : cmdId(_cmdId), responseExtension(0)
{
}

ResponseContainer::~ResponseContainer()
{
    delete responseExtension;
    for (int i = 0; i < preResponseQueue.size(); ++i)
        delete preResponseQueue[i].second;
    for (int i = 0; i < postResponseQueue.size(); ++i)
        delete postResponseQueue[i].second;
}
