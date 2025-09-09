#include "player_info.h"

PlayerInfo::PlayerInfo(const ServerInfo_User &info, int _id, bool _local, bool _judge)
    : id(_id), local(_local), judge(_judge), handVisible(false), conceded(false), zoneId(0)
{
    userInfo = new ServerInfo_User;
    userInfo->CopyFrom(info);
}

void PlayerInfo::setConceded(bool _conceded)
{
    conceded = _conceded;
    // TODO: deal with this
    /*setVisible(!conceded);
    if (conceded) {
        clear();
    }
    emit playerCountChanged();*/
}

void PlayerInfo::setZoneId(int _zoneId)
{
    // TODO: deal with this
    zoneId = _zoneId;
    // playerArea->setPlayerZoneId(_zoneId);
}