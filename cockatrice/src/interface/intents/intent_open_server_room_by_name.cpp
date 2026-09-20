#include "intent_open_server_room_by_name.h"

#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_connect_to_server.h"

#include <libcockatrice/protocol/pb/event_list_rooms.pb.h>
#include <libcockatrice/protocol/pb/response_join_room.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>

IntentOpenServerRoomByName::IntentOpenServerRoomByName(TabSupervisor *_tabSupervisor,
                                                       RemoteClient *_remoteClient,
                                                       std::unique_ptr<ContextJoinRoom> _context,
                                                       const QString &_roomName)
    : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), context(_context.release()),
      roomName(_roomName)
{
    checkTimer.setInterval(250);
    connect(&checkTimer, &QTimer::timeout, this, [this]() {
        if (selectOpenRoom()) {
            checkTimer.stop();
        }
    });
}

bool IntentOpenServerRoomByName::checkPrecondition() const
{
    if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
        return false;
    }
    // peerPort() reflects the actual TCP peer, which may differ from the
    // configured server port (e.g. when connecting through a proxy), so only
    // the hostname is compared here.
    if (remoteClient->peerName() != context->serverContext.hostname) {
        return false;
    }
    if (QString::number(remoteClient->peerPort()) != context->serverContext.port) {
        return false;
    }

    return true;
}

void IntentOpenServerRoomByName::onPreconditionSatisfied()
{
    if (listening) {
        return;
    }
    listening = true;

    if (selectOpenRoom()) {
        return;
    }

    // The room selector is the component that requests the room list, so the
    // server tab must exist for the room to be resolved by name.
    if (!tabSupervisor->getTabServer()) {
        tabSupervisor->openTabServer();
    }
    if (!tabSupervisor->getTabServer()) {
        emitFailed(tr("No server tab available"));
        return;
    }

    connect(remoteClient, &RemoteClient::listRoomsEventReceived, this, &IntentOpenServerRoomByName::processListRooms);
    connect(remoteClient, &RemoteClient::statusChanged, this, &IntentOpenServerRoomByName::onClientStatusChanged);

    // The room tab may be opened by our own join, by the room selector's auto-join, or by a
    // join that was already in flight. Poll until it shows up.
    checkTimer.start();

    // While no join has been sent yet, keep the room list fresh: the list may have been
    // requested before we subscribed to it, or a response may have been dropped during a
    // busy login burst. A stale list would otherwise leave the room unresolved forever.
    connect(&refreshTimer, &QTimer::timeout, this, [this]() {
        if (!joinPending) {
            remoteClient->sendCommand(remoteClient->prepareSessionCommand(Command_ListRooms()));
        }
    });
    refreshTimer.setInterval(5000);
    refreshTimer.start();

    // Last-resort failure for "the room genuinely is not in a fresh list". This must NOT
    // fire while a join is in flight: a loaded server may take longer than that to answer
    // during a login burst, and killing the intent early would leave the connection
    // registered in the room with no tab to display it and every later join attempt
    // would then be rejected with RespContextError.
    QTimer::singleShot(20000, this, [this]() {
        if (!joinPending) {
            emitFailed(tr("Timed out while looking for the server room %1").arg(roomName));
        }
    });
}

void IntentOpenServerRoomByName::onPreconditionNotSatisfied()
{
    runDependency(new IntentConnectToServer(remoteClient, &context->serverContext));
}

void IntentOpenServerRoomByName::onClientStatusChanged(ClientStatus status)
{
    if (status != ClientStatus::StatusLoggedIn) {
        emitFailed(tr("Disconnected while looking for the server room %1").arg(roomName));
    }
}

bool IntentOpenServerRoomByName::selectOpenRoom()
{
    const auto &roomTabs = tabSupervisor->getRoomTabs();
    for (auto i = roomTabs.cbegin(), end = roomTabs.cend(); i != end; ++i) {
        TabRoom *room = i.value();
        if (room->getRoomName() == roomName) {
            tabSupervisor->setCurrentWidget(room);
            emitFinished();
            return true;
        }
    }
    return false;
}

void IntentOpenServerRoomByName::processListRooms(const Event_ListRooms &event)
{
    if (selectOpenRoom()) {
        return;
    }

    for (int i = 0; i < event.room_list_size(); ++i) {
        const ServerInfo_Room &room = event.room_list(i);
        if (room.has_name() && QString::fromStdString(room.name()) == roomName) {
            openRoom(room);
            return;
        }
    }
}

void IntentOpenServerRoomByName::openRoom(const ServerInfo_Room &roomInfo)
{
    if (joinPending) {
        return;
    }
    joinPending = true;

    // Rooms flagged auto_join are joined by the room selector automatically. Sending our own
    // Command_JoinRoom on top of that would be answered with RespContextError.
    if (roomInfo.has_auto_join() && roomInfo.auto_join()) {
        return;
    }

    Command_JoinRoom cmd;
    cmd.set_room_id(roomInfo.room_id());
    PendingCommand *pend = remoteClient->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this,
            [this](const Response &r, const CommandContainer &, const QVariant &) { handleJoinResponse(r); });
    remoteClient->sendCommand(pend);
}

void IntentOpenServerRoomByName::handleJoinResponse(const Response &response)
{
    switch (response.response_code()) {
        case Response::RespOk: {
            const Response_JoinRoom &resp = response.GetExtension(Response_JoinRoom::ext);
            if (!tabSupervisor->getRoomTabs().contains(resp.room_info().room_id())) {
                tabSupervisor->addRoomTab(resp.room_info(), true);
            }
            emitFinished();
            return;
        }
        case Response::RespNameNotFound:
            emitFailed(tr("Failed to join the server room %1: it doesn't exist on the server.").arg(roomName));
            return;
        case Response::RespUserLevelTooLow:
            emitFailed(tr("You do not have the required permission to join the server room %1.").arg(roomName));
            return;
        case Response::RespContextError:
            // The room was already joined by someone else (e.g. the room selector's
            // auto-join). It will show up in the room tabs shortly, so keep waiting.
            return;
        default:
            emitFailed(tr("Failed to join the server room %1 due to an unknown error.").arg(roomName));
            return;
    }
}
