#ifndef COCKATRICE_PLAYER_INFO_H
#define COCKATRICE_PLAYER_INFO_H

#include "../../../common/pb/serverinfo_user.pb.h"
#include "../../deck/deck_loader.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"
#include "player_target.h"

#include <QObject>

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
    bool conceded;
    int zoneId;

    DeckLoader *deck;
    QStringList predefinedTokens;

    int getZoneId() const
    {
        return zoneId;
    }

    void setZoneId(int _zoneId);

    void setDeck(const DeckLoader &_deck);

    void setConceded(bool _conceded);
    bool getConceded() const
    {
        return conceded;
    }

    int getId() const
    {
        return id;
    }
    ServerInfo_User *getUserInfo() const
    {
        return userInfo;
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

    QStringList getPredefinedTokens() const
    {
        return predefinedTokens;
    }
};

#endif // COCKATRICE_PLAYER_INFO_H
