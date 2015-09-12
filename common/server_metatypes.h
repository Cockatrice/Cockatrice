#ifndef SERVER_METATYPES_H
#define SERVER_METATYPES_H

#include <QMetaType>

#include "pb/serverinfo_ban.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_room.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "pb/commands.pb.h"
#include "pb/response.pb.h"
#include "pb/game_event_container.pb.h"
#include "pb/isl_message.pb.h"
#include "pb/room_commands.pb.h"

Q_DECLARE_METATYPE(ServerInfo_Ban)
Q_DECLARE_METATYPE(ServerInfo_User)
Q_DECLARE_METATYPE(ServerInfo_Room)
Q_DECLARE_METATYPE(ServerInfo_Game)
Q_DECLARE_METATYPE(CommandContainer)
Q_DECLARE_METATYPE(Response)
Q_DECLARE_METATYPE(GameEventContainer)
Q_DECLARE_METATYPE(IslMessage)
Q_DECLARE_METATYPE(Command_JoinGame)

#endif
