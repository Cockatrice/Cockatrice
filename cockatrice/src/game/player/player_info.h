/**
 * @file player_info.h
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_INFO_H
#define COCKATRICE_PLAYER_INFO_H

#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"
#include "player_target.h"

#include <QObject>
#include <libcockatrice/models/deck_list/deck_loader.h>
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
    bool handVisible;

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

    void setHandVisible(bool _handVisible)
    {
        handVisible = _handVisible;
    }

    bool getHandVisible() const
    {
        return handVisible;
    }

    QString getName() const
    {
        return QString::fromStdString(userInfo->name());
    }
};

#endif // COCKATRICE_PLAYER_INFO_H
