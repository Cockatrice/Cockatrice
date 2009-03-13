#include "counterlist.h"

Counter *CounterList::findCounter(const QString &name) const
{
	for (int i = 0; i < size(); i++) {
		Counter *temp = at(i);
		if (!temp->getName().compare(name))
			return temp;
	}
	return 0;
}
