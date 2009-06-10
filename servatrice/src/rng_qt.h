#ifndef RNG_QT_H
#define RNG_QT_H

#include "abstractrng.h"

class RNG_Qt : public AbstractRNG {
	Q_OBJECT
public:
	RNG_Qt(QObject *parent = 0);
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
