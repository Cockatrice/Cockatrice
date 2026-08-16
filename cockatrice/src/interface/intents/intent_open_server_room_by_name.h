#ifndef COCKATRICE_INTENT_OPEN_SERVER_ROOM_BY_NAME_H
#define COCKATRICE_INTENT_OPEN_SERVER_ROOM_BY_NAME_H

#include "contexts/context_join_room.h"
#include "intent.h"
#include "remote_client.h"

#include <QScopedPointer>
#include <QString>
#include <QTimer>
#include <memory>

class TabRoom;
class TabSupervisor;
class Event_ListRooms;
class ServerInfo_Room;

/**
 * @brief Connects to the configured server and opens a room identified by its name.
 *
 * Room ids are assigned by the server per session, so the room is resolved by name from the
 * room list once the client is logged in. If the room is already open it is simply selected.
 *
 * The join itself is sent directly through the client instead of `TabServer::joinRoom`, so a
 * failed join only fails the intent silently instead of popping a modal error box during
 * startup. Success is routed to `TabSupervisor::addRoomTab`, the same tab-creation machinery
 * the normal join flow uses.
 */
class IntentOpenServerRoomByName : public Intent
{
    Q_OBJECT

public:
    IntentOpenServerRoomByName(TabSupervisor *_tabSupervisor,
                               RemoteClient *_remoteClient,
                               std::unique_ptr<ContextJoinRoom> _context,
                               const QString &_roomName);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    void processListRooms(const Event_ListRooms &event);
    void openRoom(const ServerInfo_Room &roomInfo);
    void handleJoinResponse(const Response &response);
    void onClientStatusChanged(ClientStatus status);
    bool selectOpenRoom();

    TabSupervisor *tabSupervisor;
    RemoteClient *remoteClient;
    QScopedPointer<ContextJoinRoom> context;
    QString roomName;
    bool listening = false;
    bool joinPending = false;
    QTimer checkTimer;
    QTimer refreshTimer;
};

#endif // COCKATRICE_INTENT_OPEN_SERVER_ROOM_BY_NAME_H
