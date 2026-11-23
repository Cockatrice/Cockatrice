/**
 * @file player_info.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_PLAYER_INFO_H
#define COCKATRICE_PLAYER_INFO_H

#include <QString>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

struct PlayerInfo
{
    ServerInfo_User userInfo; ///< Server userInfo of the player
    int id;                   ///< In-game playerId (shared with server)
    bool local;               ///< Whether the client is controlling this player
    bool judge;               ///< Whether this player is a judge

    bool isLocalOrJudge() const
    {
        return local || judge;
    }

    QString getName() const
    {
        return QString::fromStdString(userInfo.name());
    }
};

#endif // COCKATRICE_PLAYER_INFO_H
