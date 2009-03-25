#ifndef ZONELIST_H
#define ZONELIST_H

#include "cardzone.h"
#include <QList>

class ZoneList : public QList<CardZone *> {
public:
	CardZone *findZone(const QString &name) const;
};

#endif
