#ifndef COUNTERLIST_H
#define COUNTERLIST_H

#include "counter.h"
#include <QList>

class CounterList : public QList<Counter *> {
public:
	Counter *findCounter(const QString &name) const;
};

#endif
