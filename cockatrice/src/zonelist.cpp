#include "zonelist.h"

PlayerZone *ZoneList::findZone(const QString &name) const
{
	for (int i = 0; i < size(); i++) {
		PlayerZone *temp = at(i);
		if (!temp->getName().compare(name))
			return temp;
	}
	return 0;
}
