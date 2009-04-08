#include "random.h"
#include <QThread>

void Random::init()
{
	if (initialized)
		return;
	int seed = QDateTime::currentDateTime().toTime_t();
	qDebug(QString("%1: qsrand(%2)").arg(thread()->metaObject()->className()).arg(seed).toLatin1());
	qsrand(seed);
	initialized = true;
}

unsigned int Random::getNumber(unsigned int min, unsigned int max)
{
	int r = qrand();
	return min + (unsigned int) (((double) (max + 1 - min)) * r / (RAND_MAX + 1.0));
}
