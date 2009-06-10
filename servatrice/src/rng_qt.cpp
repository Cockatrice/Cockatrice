#include "rng_qt.h"
#include <QDateTime>
#include <stdlib.h>

RNG_Qt::RNG_Qt(QObject *parent)
	: AbstractRNG(parent)
{
	int seed = QDateTime::currentDateTime().toTime_t();
	qDebug(QString("qsrand(%1)").arg(seed).toLatin1());
	qsrand(seed);
}

unsigned int RNG_Qt::getNumber(unsigned int min, unsigned int max)
{
	int r = qrand();
	return min + (unsigned int) (((double) (max + 1 - min)) * r / (RAND_MAX + 1.0));
}
