#include "random.h"
#include <QDateTime>
#include <stdlib.h>

Random::Random(QObject *parent)
	: QObject(parent)
{
	int seed = QDateTime::currentDateTime().toTime_t();
	qDebug(QString("qsrand(%1)").arg(seed).toLatin1());
	qsrand(seed);
}

unsigned int Random::getNumber(unsigned int min, unsigned int max)
{
	int r = qrand();
	return min + (unsigned int) (((double) (max + 1 - min)) * r / (RAND_MAX + 1.0));
}
