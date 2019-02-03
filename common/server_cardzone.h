/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SERVER_CARDZONE_H
#define SERVER_CARDZONE_H

#include "pb/serverinfo_zone.pb.h"
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>

class Server_Card;
class Server_Player;
class Server_Game;
class GameEventStorage;

class Server_CardZone
{
private:
    Server_Player *player;
    QString name;
    bool has_coords;
    ServerInfo_Zone::ZoneType type;
    int cardsBeingLookedAt;
    QSet<int> playersWithWritePermission;
    bool alwaysRevealTopCard;
    QList<Server_Card *> cards;
    QMap<int, QMap<int, Server_Card *>> coordinateMap; // y -> (x -> card)
    QMap<int, QMultiMap<QString, int>> freePilesMap;   // y -> (cardName -> x)
    QMap<int, int> freeSpaceMap;                       // y -> x
    void removeCardFromCoordMap(Server_Card *card, int oldX, int oldY);
    void insertCardIntoCoordMap(Server_Card *card, int x, int y);

public:
    Server_CardZone(Server_Player *_player, const QString &_name, bool _has_coords, ServerInfo_Zone::ZoneType _type);
    ~Server_CardZone();

    const QList<Server_Card *> &getCards() const
    {
        return cards;
    }
    int removeCard(Server_Card *card);
    Server_Card *getCard(int id, int *position = nullptr, bool remove = false);

    int getCardsBeingLookedAt() const
    {
        return cardsBeingLookedAt;
    }
    void setCardsBeingLookedAt(int _cardsBeingLookedAt)
    {
        cardsBeingLookedAt = _cardsBeingLookedAt;
    }
    bool hasCoords() const
    {
        return has_coords;
    }
    ServerInfo_Zone::ZoneType getType() const
    {
        return type;
    }
    QString getName() const
    {
        return name;
    }
    Server_Player *getPlayer() const
    {
        return player;
    }
    void getInfo(ServerInfo_Zone *info, Server_Player *playerWhosAsking, bool omniscient);

    int getFreeGridColumn(int x, int y, const QString &cardName, bool dontStackSameName) const;
    bool isColumnEmpty(int x, int y) const;
    bool isColumnStacked(int x, int y) const;
    void fixFreeSpaces(GameEventStorage &ges);
    void moveCardInRow(GameEventStorage &ges, Server_Card *card, int x, int y);
    void insertCard(Server_Card *card, int x, int y);
    void updateCardCoordinates(Server_Card *card, int oldX, int oldY);
    void shuffle(int start = 0, int end = -1);
    void clear();
    void addWritePermission(int playerId);
    const QSet<int> &getPlayersWithWritePermission() const
    {
        return playersWithWritePermission;
    }
    bool getAlwaysRevealTopCard() const
    {
        return alwaysRevealTopCard;
    }
    void setAlwaysRevealTopCard(bool _alwaysRevealTopCard)
    {
        alwaysRevealTopCard = _alwaysRevealTopCard;
    }
};

#endif
