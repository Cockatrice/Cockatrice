#ifndef PLAYERLIST_H
#define PLAYERLIST_H

#include "player.h"
#include <QList>

class PlayerList : public QList<Player *> {
public:
	Player *findPlayer(int id) const;
};

#endif
