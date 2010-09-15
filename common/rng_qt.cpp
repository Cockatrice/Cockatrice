#include "rng_qt.h"
#include <QDateTime>
#include <stdlib.h>

RNG_Qt::RNG_Qt(QObject *parent)
	: RNG_Abstract(parent)
{
	int seed = QDateTime::currentDateTime().toTime_t();
	qsrand(seed);
}

unsigned int RNG_Qt::getNumber(unsigned int min, unsigned int max)
{
	int r = qrand();
	return min + (unsigned int) (((double) (max + 1 - min)) * r / (RAND_MAX + 1.0));
}
