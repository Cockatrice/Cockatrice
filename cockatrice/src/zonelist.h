#ifndef ZONELIST_H
#define ZONELIST_H

#include "playerzone.h"
#include <QList>

class ZoneList : public QList<PlayerZone *> {
public:
	PlayerZone *findZone(const QString &name) const;
};

#endif
