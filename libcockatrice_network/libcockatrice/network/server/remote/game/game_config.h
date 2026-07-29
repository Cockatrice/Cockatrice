#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <QList>
#include <QString>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class GameConfig
{
public:
    ServerInfo_User creatorInfo;
    int gameId = -1;
    QString description;
    QString password;
    int maxPlayers = 2;
    QList<int> gameTypes;
    bool onlyBuddies = false;
    bool onlyRegistered = false;
    bool spectatorsAllowed = false;
    bool spectatorsNeedPassword = false;
    bool spectatorsCanTalk = true;
    bool spectatorsSeeEverything = true;
    int startingLifeTotal = 20;
    bool shareDecklistsOnLoad = true;
};

#endif
