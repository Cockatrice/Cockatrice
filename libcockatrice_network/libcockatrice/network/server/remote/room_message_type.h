#ifndef ROOM_MESSAGE_TYPE_H
#define ROOM_MESSAGE_TYPE_H

#ifdef Q_OS_MACOS
// avoid collision from Mac OS X's ConditionalMacros.h
// https://github.com/protocolbuffers/protobuf/issues/119
#undef TYPE_BOOL
#endif
#include <QFlags>
#include <libcockatrice/protocol/pb/event_room_say.pb.h>

Q_DECLARE_FLAGS(RoomMessageTypeFlags, Event_RoomSay::RoomMessageType)
Q_DECLARE_OPERATORS_FOR_FLAGS(RoomMessageTypeFlags)

#endif
