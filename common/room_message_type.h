#ifndef ROOM_MESSAGE_TYPE_H
#define ROOM_MESSAGE_TYPE_H

#ifdef Q_OS_OSX
// avoid collision from Mac OS X's ConditionalMacros.h
// https://code.google.com/p/protobuf/issues/detail?id=119
#undef TYPE_BOOL
#endif
#include "pb/event_room_say.pb.h"
#include <QFlags>

Q_DECLARE_FLAGS(RoomMessageTypeFlags, Event_RoomSay::RoomMessageType)
Q_DECLARE_OPERATORS_FOR_FLAGS(RoomMessageTypeFlags)

#endif
