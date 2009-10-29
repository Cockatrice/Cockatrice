#ifndef RNG_QT_H
#define RNG_QT_H

#include "rng_abstract.h"

class RNG_Qt : public RNG_Abstract {
	Q_OBJECT
public:
	RNG_Qt(QObject *parent = 0);
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
