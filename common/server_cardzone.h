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
#include <QPointer>
#include <QSet>
#include <QString>

class Server_Card;
class Server_Player;
class Server_Game;
class GameEventStorage;
class ZoneConfig;
class ZoneRef;

class Server_CardZone : public std::enable_shared_from_this<Server_CardZone>
{
public:
    enum ZoneFlag
    {
        NoFlag = 0x0,
        HasCoords = 0x1 << 0,  // having coords means this zone has x and y coordinates
        CanShuffle = 0x1 << 1, // enable the "shuffle" command
        AlwaysLookAtTopCard = 0x1 << 2,
        AlwaysRevealTopCard = 0x1 << 3,
        DynamicZone = 0x1 << 4, // dynamic zones can be destroyed
    };
    Q_DECLARE_FLAGS(ZoneFlags, ZoneFlag);

    std::shared_ptr<Server_CardZone> getPtr()
    {
        return shared_from_this();
    }

    [[nodiscard]] static std::shared_ptr<Server_CardZone>
    create(Server_Player *player, const QString &name, ZoneType type, ZoneFlags flags = {})
    {
        return std::shared_ptr<Server_CardZone>(new Server_CardZone(player, name, type, flags));
    }

private:
    Server_CardZone(Server_Player *_player, const QString &_name, ZoneType _type, ZoneFlags flags);

    Server_Player *player;
    QString name;
    ZoneType type;
    int cardsBeingLookedAt;
    QSet<int> playersWithWritePermission;
    QList<Server_Card *> cards;
    QMap<int, QMap<int, Server_Card *>> coordinateMap; // y -> (x -> card)
    QMap<int, QMultiMap<QString, int>> freePilesMap;   // y -> (cardName -> x)
    QMap<int, int> freeSpaceMap;                       // y -> x
    ZoneFlags flags;
    QPointer<Server_Card> parentCard;
    void removeCardFromCoordMap(Server_Card *card, int oldX, int oldY);
    void insertCardIntoCoordMap(Server_Card *card, int x, int y);

public:
    ~Server_CardZone();

    const QList<Server_Card *> &getCards() const
    {
        return cards;
    }
    int removeCard(Server_Card *card);
    int removeCard(Server_Card *card, bool &wasLookedAt);
    Server_Card *getCard(int id, int *position = nullptr, bool remove = false);

    int getCardsBeingLookedAt() const
    {
        return cardsBeingLookedAt;
    }
    void setCardsBeingLookedAt(int _cardsBeingLookedAt)
    {
        cardsBeingLookedAt = qMax(0, _cardsBeingLookedAt);
    }
    bool isCardAtPosLookedAt(int pos) const;
    bool hasCoords() const
    {
        return flags.testFlag(HasCoords);
    }
    bool canShuffle() const
    {
        return flags.testFlag(CanShuffle);
    }
    bool isDynamic() const
    {
        return flags.testFlag(DynamicZone);
    }
    ZoneType getType() const
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
    void insertCard(Server_Card *card, int x, int y, bool visible = false);
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
        return flags.testFlag(AlwaysRevealTopCard);
    }
    void setAlwaysRevealTopCard(bool _alwaysRevealTopCard)
    {
        flags.setFlag(AlwaysRevealTopCard, _alwaysRevealTopCard);
    }
    bool getAlwaysLookAtTopCard() const
    {
        return flags.testFlag(AlwaysLookAtTopCard);
    }
    void setAlwaysLookAtTopCard(bool _alwaysLookAtTopCard)
    {
        flags.setFlag(AlwaysLookAtTopCard, _alwaysLookAtTopCard);
    }
    void attachToCard(Server_Card *card);
    void detach()
    {
        attachToCard(nullptr);
    }

    void copyConfig(ZoneConfig *config);
    void copyRef(ZoneRef *ref);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Server_CardZone::ZoneFlags);

#endif
