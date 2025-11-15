#include "server_abstractuserinterface.h"

#include "game/server_game.h"
#include "game/server_player.h"
#include "server.h"
#include "server_player_reference.h"
#include "server_response_containers.h"
#include "server_room.h"

#include <QDebug>
#include <QList>
#include <QPair>
#include <google/protobuf/descriptor.h>
#include <libcockatrice/protocol/pb/event_game_joined.pb.h>
#include <libcockatrice/protocol/pb/event_game_state_changed.pb.h>

void Server_AbstractUserInterface::sendProtocolItemByType(ServerMessage::MessageType type,
                                                          const ::google::protobuf::Message &item)
{
    switch (type) {
        case ServerMessage::RESPONSE:
            sendProtocolItem(static_cast<const Response &>(item));
            break;
        case ServerMessage::SESSION_EVENT:
            sendProtocolItem(static_cast<const SessionEvent &>(item));
            break;
        case ServerMessage::GAME_EVENT_CONTAINER:
            sendProtocolItem(static_cast<const GameEventContainer &>(item));
            break;
        case ServerMessage::ROOM_EVENT:
            sendProtocolItem(static_cast<const RoomEvent &>(item));
            break;
    }
}

SessionEvent *Server_AbstractUserInterface::prepareSessionEvent(const ::google::protobuf::Message &sessionEvent)
{
    auto *event = new SessionEvent;
    event->GetReflection()
        ->MutableMessage(event, sessionEvent.GetDescriptor()->FindExtensionByName("ext"))
        ->CopyFrom(sessionEvent);
    return event;
}

void Server_AbstractUserInterface::sendResponseContainer(const ResponseContainer &responseContainer,
                                                         Response::ResponseCode responseCode)
{
    const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *>> &preResponseQueue =
        responseContainer.getPreResponseQueue();
    for (int i = 0; i < preResponseQueue.size(); ++i)
        sendProtocolItemByType(preResponseQueue[i].first, *preResponseQueue[i].second);

    if (responseCode != Response::RespNothing) {
        Response response;
        response.set_cmd_id(responseContainer.getCmdId());
        response.set_response_code(responseCode);
        ::google::protobuf::Message *responseExtension = responseContainer.getResponseExtension();
        if (responseExtension)
            response.GetReflection()
                ->MutableMessage(&response, responseExtension->GetDescriptor()->FindExtensionByName("ext"))
                ->CopyFrom(*responseExtension);
        sendProtocolItem(response);
    }

    const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *>> &postResponseQueue =
        responseContainer.getPostResponseQueue();
    for (int i = 0; i < postResponseQueue.size(); ++i)
        sendProtocolItemByType(postResponseQueue[i].first, *postResponseQueue[i].second);
}

void Server_AbstractUserInterface::playerRemovedFromGame(Server_Game *game)
{
    qDebug() << "Server_AbstractUserInterface::playerRemovedFromGame(): gameId =" << game->getGameId();

    QMutexLocker locker(&gameListMutex);
    games.remove(game->getGameId());
}

void Server_AbstractUserInterface::playerAddedToGame(int gameId, int roomId, int playerId)
{
    qDebug() << "Server_AbstractUserInterface::playerAddedToGame(): gameId =" << gameId;

    QMutexLocker locker(&gameListMutex);
    games.insert(gameId, QPair<int, int>(roomId, playerId));
}

void Server_AbstractUserInterface::joinPersistentGames(ResponseContainer &rc)
{
    QList<PlayerReference> gamesToJoin =
        server->getPersistentPlayerReferences(QString::fromStdString(userInfo->name()));

    server->roomsLock.lockForRead();
    for (int i = 0; i < gamesToJoin.size(); ++i) {
        const PlayerReference &pr = gamesToJoin.at(i);

        Server_Room *room = server->getRooms().value(pr.getRoomId());
        if (!room)
            continue;
        QReadLocker roomGamesLocker(&room->gamesLock);

        Server_Game *game = room->getGames().value(pr.getGameId());
        if (!game)
            continue;
        QMutexLocker gameLocker(&game->gameMutex);

        auto *participant = game->getParticipants().value(pr.getPlayerId());
        if (!participant)
            continue;

        participant->setUserInterface(this);
        playerAddedToGame(game->getGameId(), room->getId(), participant->getPlayerId());

        game->createGameJoinedEvent(participant, rc, true);
    }
    server->roomsLock.unlock();
}
