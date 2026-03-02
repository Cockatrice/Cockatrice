#include "player_info.h"

PlayerInfo::PlayerInfo(const ServerInfo_User &info, int _id, bool _local, bool _judge)
    : id(_id), local(_local), judge(_judge), handVisible(false)
{
    userInfo = new ServerInfo_User;
    userInfo->CopyFrom(info);
}