#include "playerlist.h"

Player *PlayerList::findPlayer(int id) const
{
	for (int i = 0; i < size(); i++) {
		Player *temp = at(i);
		if (temp->getId() == id)
			return temp;
	}
	return 0;
}
