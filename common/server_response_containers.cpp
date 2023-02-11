#include "server_response_containers.h"

#include "server_game.h"

#include <google/protobuf/descriptor.h>

GameEventBuilder::GameEventBuilder(const ::google::protobuf::Message &event, int playerId)
{
    publicEvent.GetReflection()
        ->MutableMessage(&publicEvent, event.GetDescriptor()->FindExtensionByName("ext"))
        ->CopyFrom(event);
    publicEvent.set_player_id(playerId);
    privateEvent.set_player_id(playerId);
}

GameEventBuilder GameEventBuilder::withPrivateEvent(int playerId, const ::google::protobuf::Message &event)
{
    GameEventBuilder other(*this);
    other.privatePlayerId = playerId;
    other.privateEvent.GetReflection()
        ->MutableMessage(&other.privateEvent, event.GetDescriptor()->FindExtensionByName("ext"))
        ->CopyFrom(event);

    return other;
}

const GameEventContainer &GameEvents::getEventsForPlayer(int playerId) const
{
    auto it = privateEvents.find(playerId);
    if (it == privateEvents.end()) {
        return publicEvents;
    }

    return it.value();
}

void GameEvents::setGameEventContext(const GameEventContext &gameEventContext)
{
    publicEvents.mutable_context()->CopyFrom(gameEventContext);
    replayEvents.mutable_context()->CopyFrom(gameEventContext);

    for (auto &privateContainer : privateEvents) {
        privateContainer.mutable_context()->CopyFrom(gameEventContext);
    }
}

void GameEvents::setForcedByJudge(int judgeId)
{
    publicEvents.set_forced_by_judge(judgeId);
    replayEvents.set_forced_by_judge(judgeId);

    for (auto &privateContainer : privateEvents) {
        privateContainer.set_forced_by_judge(judgeId);
    }
}

void GameEvents::setGameId(int gameId)
{
    publicEvents.set_game_id(gameId);
    replayEvents.set_game_id(gameId);

    for (auto &privateContainer : privateEvents) {
        privateContainer.set_game_id(gameId);
    }
}

void GameEventsBuilder::append(const GameEventBuilder &builder)
{
    gameEvents.append(builder);

    for (auto &upgrade : upgrades) {
        upgrade.second.append(builder);
    }
}

void GameEventsBuilder::append(const GameEventsBuilder &other)
{
    QList<std::pair<QSet<QString>, QList<GameEventBuilder>>> newUpgrades;

    // When appending two builders that have upgrades U1, U2, ... and V1, V2, ...
    // the event list for a client with a given feature-set is: the last U that
    // is supported followed by the last V that is supported.
    //
    // This is obtained by the following featureset order:
    //  V1, V2, ...                 [none of the Us match]
    //  U1, U1 + V1, U1 + V2, ...   [U1 matches, try all the possible Vs in order]
    //  U2, U2 + V1, U2 + V2, ...   [U2 matches, try all the possible Vs in order]

    for (const auto &oupgrade : other.upgrades) {
        newUpgrades.append({oupgrade.first, gameEvents + oupgrade.second});
    }

    for (const auto &upgrade : upgrades) {
        newUpgrades.append({upgrade.first, upgrade.second + other.gameEvents});

        for (const auto &oupgrade : other.upgrades) {
            newUpgrades.append({upgrade.first | oupgrade.first, upgrade.second + oupgrade.second});
        }
    }

    // The simple union algorithm above can create useless event lists. For
    // instance, if we append a builder:
    //
    //  gameEvents: defaults1
    //  upgrades: [A -> events1]
    //
    // with another builder:
    //
    //  gameEvents: defaults2
    //  upgrades: [A -> events2]
    //
    // we will get a combined upgrades of:
    //
    //  [A -> defaults1 + events2, A -> events1 + defaults2, A -> events1 + events2]
    //
    // The upgrade resolution protocol will always select the set events1 +
    // events2, so we can safely ignore the previous ones.
    QSet<QSet<QString>> seenFeatureSets;
    upgrades.clear();
    for (auto it = newUpgrades.rbegin(), end = newUpgrades.rend(); it != end; ++it) {
        auto seenIt = std::find_if(seenFeatureSets.begin(), seenFeatureSets.end(),
                                   [it](const QSet<QString> &featureSet) { return it->first.contains(featureSet); });
        if (seenIt == seenFeatureSets.end()) {
            upgrades.prepend(*it);
            seenFeatureSets.insert(it->first);
        }
    }

    // Don't forget the case where there are no upgrades ;)
    gameEvents.append(other.gameEvents);
}

void GameEventsBuilder::upgrade(const QSet<QString> &features, const GameEventsBuilder &other)
{
    upgrades.append({features, other.gameEvents});

    for (const auto &pair : other.upgrades) {
        upgrades.append({features + pair.first, pair.second});
    }
}

GameEvents GameEventsBuilder::build() const
{
    GameEventContainer publicContainer, replayContainer;

    // Ensure we have a private container for each private player.
    //
    // Note that we must take into consideration all possible feature flags, in
    // the unlikely case that private events appear behind a feature flag but
    // not in the default stream.
    QMap<int, GameEventContainer> privateContainers;
    for (const auto &eventBuilder : gameEvents) {
        if (eventBuilder.getPrivatePlayerId() != -1) {
            privateContainers.insert(eventBuilder.getPrivatePlayerId(), {});
        }
    }

    for (const auto &pair : upgrades) {
        for (const auto &eventBuilder : pair.second) {
            if (eventBuilder.getPrivatePlayerId() != -1) {
                privateContainers.insert(eventBuilder.getPrivatePlayerId(), {});
            }
        }
    }

    // Build the legacy containers
    for (const auto &eventBuilder : gameEvents) {
        publicContainer.add_event_list()->CopyFrom(eventBuilder.getPublicEvent());
        replayContainer.add_event_list()->CopyFrom(eventBuilder.getReplayEvent());

        for (auto it = privateContainers.begin(), end = privateContainers.end(); it != end; ++it) {
            it.value().add_event_list()->CopyFrom(eventBuilder.getEventForPlayer(it.key()));
        }
    }

    // Add the upgrades
    for (const auto &pair : upgrades) {
        auto *publicUpgrade = publicContainer.add_upgrades();
        for (const QString &featureName : pair.first) {
            publicUpgrade->add_features(featureName.toStdString());
        }

        auto *replayUpgrade = replayContainer.add_upgrades();
        replayUpgrade->mutable_features()->CopyFrom(publicUpgrade->features());

        for (const auto &eventBuilder : pair.second) {
            publicUpgrade->add_events()->CopyFrom(eventBuilder.getPublicEvent());
            replayUpgrade->add_events()->CopyFrom(eventBuilder.getReplayEvent());
        }

        for (auto it = privateContainers.begin(), end = privateContainers.end(); it != end; ++it) {
            auto *upgrade = it.value().add_upgrades();
            upgrade->mutable_features()->CopyFrom(publicUpgrade->features());

            for (const auto &eventBuilder : pair.second) {
                upgrade->add_events()->CopyFrom(eventBuilder.getEventForPlayer(it.key()));
            }
        }
    }

    return {publicContainer, replayContainer, privateContainers};
}

GameEventStorage::GameEventStorage() : gameEventContext(nullptr)
{
}

GameEventStorage::~GameEventStorage()
{
    delete gameEventContext;
}

void GameEventStorage::setGameEventContext(const ::google::protobuf::Message &_gameEventContext)
{
    delete gameEventContext;
    gameEventContext = new GameEventContext;
    gameEventContext->GetReflection()
        ->MutableMessage(gameEventContext, _gameEventContext.GetDescriptor()->FindExtensionByName("ext"))
        ->CopyFrom(_gameEventContext);
}

void GameEventStorage::enqueueGameEvent(const ::google::protobuf::Message &event, int playerId)
{
    enqueueGameEvent({event, playerId});
}

void GameEventStorage::enqueueGameEvent(const GameEventBuilder &builder)
{
    gameEventsBuilder.append(builder);
}

void GameEventStorage::enqueueGameEvents(const GameEventsBuilder &builder)
{
    gameEventsBuilder.append(builder);
}

void GameEventStorage::sendToGame(Server_Game *game)
{
    if (!gameEventsBuilder.hasEvents())
        return;

    GameEvents gameEvents = gameEventsBuilder.build();

    if (gameEventContext) {
        gameEvents.setGameEventContext(*gameEventContext);
    }

    if (forcedByJudge != -1) {
        gameEvents.setForcedByJudge(forcedByJudge);
    }

    game->sendGameEvents(std::move(gameEvents));
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
