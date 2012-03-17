#ifndef RNG_SFMT_H
#define RNG_SFMT_H

#include "rng_abstract.h"
#include <QMutex>

class RNG_SFMT : public RNG_Abstract {
	Q_OBJECT
private:
	QMutex mutex;
public:
	RNG_SFMT(QObject *parent = 0);
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
 
