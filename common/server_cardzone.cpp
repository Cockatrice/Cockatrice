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
#include "server_cardzone.h"
#include "server_card.h"
#include "server_player.h"
#include "rng_abstract.h"
#include <QSet>
#include <QDebug>
#include "pb/command_move_card.pb.h"

Server_CardZone::Server_CardZone(Server_Player *_player, const QString &_name, bool _has_coords, ServerInfo_Zone::ZoneType _type)
    : player(_player),
          name(_name),
          has_coords(_has_coords),
          type(_type),
          cardsBeingLookedAt(0),
          alwaysRevealTopCard(false)
{
}

Server_CardZone::~Server_CardZone()
{
    qDebug() << "Server_CardZone destructor:" << name;
    clear();
}

void Server_CardZone::shuffle()
{
    // Size 0 or 1 decks are sorted
    if (cards.size() < 2) return;
    for (int i = cards.size() - 1; i > 0; i--){
        int j = rng->rand(0, i);
        cards.swap(j,i);
    }
    playersWithWritePermission.clear();
}


void Server_CardZone::removeCardFromCoordMap(Server_Card *card, int oldX, int oldY)
{
    if (oldX < 0)
        return;
    
    const int baseX = (oldX / 3) * 3;
    QMap<int, Server_Card *> &coordMap = coordinateMap[oldY];
    
    if (coordMap.contains(baseX) && coordMap.contains(baseX + 1) && coordMap.contains(baseX + 2))
        // If the removal of this card has opened up a previously full pile...
        freePilesMap[oldY].insert(coordMap.value(baseX)->getName(), baseX);
    
    coordMap.remove(oldX);
    
    if (!(coordMap.contains(baseX) && coordMap.value(baseX)->getName() == card->getName()) && !(coordMap.contains(baseX + 1) && coordMap.value(baseX + 1)->getName() == card->getName()) && !(coordMap.contains(baseX + 2) && coordMap.value(baseX + 2)->getName() == card->getName()))
        // If this card was the last one with this name...
        freePilesMap[oldY].remove(card->getName(), baseX);
    
    if (!coordMap.contains(baseX) && !coordMap.contains(baseX + 1) && !coordMap.contains(baseX + 2)) {
        // If the removal of this card has freed a whole pile, i.e. it was the last card in it...
        if (baseX < freeSpaceMap[oldY])
            freeSpaceMap[oldY] = baseX;
    }
}

void Server_CardZone::insertCardIntoCoordMap(Server_Card *card, int x, int y)
{
    if (x < 0)
        return;
    
    coordinateMap[y].insert(x, card);
    if (!(x % 3)) {
        if (!card->getFaceDown() && !freePilesMap[y].contains(card->getName(), x) && card->getAttachedCards().isEmpty())
            freePilesMap[y].insert(card->getName(), x);
        if (freeSpaceMap[y] == x) {
            int nextFreeX = x;
            do {
                nextFreeX += 3;
            } while (coordinateMap[y].contains(nextFreeX) || coordinateMap[y].contains(nextFreeX + 1) || coordinateMap[y].contains(nextFreeX + 2));
            freeSpaceMap[y] = nextFreeX;
        }
    } else if (!((x - 2) % 3)) {
        const int baseX = (x / 3) * 3;
        freePilesMap[y].remove(coordinateMap[y].value(baseX)->getName(), baseX);
    }
}

int Server_CardZone::removeCard(Server_Card *card)
{
    int index = cards.indexOf(card);
    cards.removeAt(index);
    if (has_coords)
        removeCardFromCoordMap(card, card->getX(), card->getY());
    card->setZone(0);
    
    return index;
}

Server_Card *Server_CardZone::getCard(int id, int *position, bool remove)
{
    if (type != ServerInfo_Zone::HiddenZone) {
        for (int i = 0; i < cards.size(); ++i) {
            Server_Card *tmp = cards[i];
            if (tmp->getId() == id) {
                if (position)
                    *position = i;
                if (remove) {
                    cards.removeAt(i);
                    tmp->setZone(0);
                }
                return tmp;
            }
        }
        return NULL;
    } else {
        if ((id >= cards.size()) || (id < 0))
            return NULL;
        Server_Card *tmp = cards[id];
        if (position)
            *position = id;
        if (remove) {
            cards.removeAt(id);
            tmp->setZone(0);
        }
        return tmp;
    }
}

int Server_CardZone::getFreeGridColumn(int x, int y, const QString &cardName, bool dontStackSameName) const
{
    const QMap<int, Server_Card *> &coordMap = coordinateMap.value(y);
    if (x == -1) {
        if (!dontStackSameName && freePilesMap[y].contains(cardName)) {
            x = (freePilesMap[y].value(cardName) / 3) * 3;
            
            if(coordMap.contains(x) && 
                (coordMap[x]->getFaceDown() || 
                    !coordMap[x]->getAttachedCards().isEmpty())) {
                // don't pile up on: 1. facedown cards 2. cards with attached cards
            } else if (!coordMap.contains(x))
                return x;
            else if (!coordMap.contains(x + 1))
                return x + 1;
            else
                return x + 2;
        }
    } else if (x >= 0) {
        int resultX = 0;
        x = (x / 3) * 3;
        if (!coordMap.contains(x))
            resultX = x;
        else if (!coordMap.value(x)->getAttachedCards().isEmpty()) {
            resultX = x;
            x = -1;
        } else if (!coordMap.contains(x + 1))
            resultX = x + 1;
        else if (!coordMap.contains(x + 2))
            resultX = x + 2;
        else {
            resultX = x;
            x = -1;
        }
        if (x < 0)
            while (coordMap.contains(resultX))
                resultX += 3;

        return resultX;
    }
    
    return freeSpaceMap[y];
}

bool Server_CardZone::isColumnStacked(int x, int y) const
{
    if (!has_coords)
        return false;
    
    return coordinateMap[y].contains((x / 3) * 3 + 1);
}

bool Server_CardZone::isColumnEmpty(int x, int y) const
{
    if (!has_coords)
        return true;
    
    return !coordinateMap[y].contains((x / 3) * 3);
}

void Server_CardZone::moveCardInRow(GameEventStorage &ges, Server_Card *card, int x, int y)
{
    CardToMove *cardToMove = new CardToMove;
    cardToMove->set_card_id(card->getId());
    player->moveCard(ges, this, QList<const CardToMove *>() << cardToMove, this, x, y, false, false);
    delete cardToMove;
}

void Server_CardZone::fixFreeSpaces(GameEventStorage &ges)
{
    if (!has_coords)
        return;
    
    QSet<QPair<int, int> > placesToLook;
    for (int i = 0; i < cards.size(); ++i)
        placesToLook.insert(QPair<int, int>((cards[i]->getX() / 3) * 3, cards[i]->getY()));
    
    QSetIterator<QPair<int, int> > placeIterator(placesToLook);
    while (placeIterator.hasNext()) {
        const QPair<int, int> &foo = placeIterator.next();
        int baseX = foo.first;
        int y = foo.second;
        
        if (!coordinateMap[y].contains(baseX)) {
            if (coordinateMap[y].contains(baseX + 1))
                moveCardInRow(ges, coordinateMap[y].value(baseX + 1), baseX, y);
            else if (coordinateMap[y].contains(baseX + 2)) {
                moveCardInRow(ges, coordinateMap[y].value(baseX + 2), baseX, y);
                continue;
            } else
                continue;
        }
        if (!coordinateMap[y].contains(baseX + 1) && coordinateMap[y].contains(baseX + 2))
            moveCardInRow(ges, coordinateMap[y].value(baseX + 2), baseX + 1, y);
    }
}

void Server_CardZone::updateCardCoordinates(Server_Card *card, int oldX, int oldY)
{
    if (!has_coords)
        return;
    
    if (oldX != -1)
        removeCardFromCoordMap(card, oldX, oldY);
    insertCardIntoCoordMap(card, card->getX(), card->getY());
}

void Server_CardZone::insertCard(Server_Card *card, int x, int y)
{
    if (hasCoords()) {
        card->setCoords(x, y);
        cards.append(card);
        insertCardIntoCoordMap(card, x, y);
    } else {
        card->setCoords(0, 0);
        if (x == -1)
            cards.append(card);
        else
            cards.insert(x, card);
    }
    card->setZone(this);
}

void Server_CardZone::clear()
{
    for (int i = 0; i < cards.size(); i++)
        delete cards.at(i);
    cards.clear();
    coordinateMap.clear();
    freePilesMap.clear();
    freeSpaceMap.clear();
    playersWithWritePermission.clear();
}

void Server_CardZone::addWritePermission(int playerId)
{
    playersWithWritePermission.insert(playerId);
}

void Server_CardZone::getInfo(ServerInfo_Zone *info, Server_Player *playerWhosAsking, bool omniscient)
{
    info->set_name(name.toStdString());
    info->set_type(type);
    info->set_with_coords(has_coords);
    info->set_card_count(cards.size());
    info->set_always_reveal_top_card(alwaysRevealTopCard);
    if (
        (((playerWhosAsking == player) || omniscient) && (type != ServerInfo_Zone::HiddenZone))
        || ((playerWhosAsking != player) && (type == ServerInfo_Zone::PublicZone))
    ) {
        QListIterator<Server_Card *> cardIterator(cards);
        while (cardIterator.hasNext())
            cardIterator.next()->getInfo(info->add_card_list());
    }
}
