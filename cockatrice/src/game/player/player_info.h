/**
 * @file player_info.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_PLAYER_INFO_H
#define COCKATRICE_PLAYER_INFO_H

#include "player_target.h"

#include <QObject>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class PlayerInfo : public QObject
{
    Q_OBJECT
public:
    PlayerInfo(const ServerInfo_User &info, int id, bool local, bool judge);

    ServerInfo_User *userInfo;
    int id;
    bool local;
    bool judge;

    int getId() const
    {
        return id;
    }
    ServerInfo_User *getUserInfo() const
    {
        return userInfo;
    }

    void setLocal(bool _local)
    {
        local = _local;
    }

    bool getLocal() const
    {
        return local;
    }
    bool getLocalOrJudge() const
    {
        return local || judge;
    }
    bool getJudge() const
    {
        return judge;
    }

    QString getName() const
    {
        return QString::fromStdString(userInfo->name());
    }
};

#endif // COCKATRICE_PLAYER_INFO_H
