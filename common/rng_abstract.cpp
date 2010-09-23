#include "rng_abstract.h"
#include <QDebug>


QVector<int> RNG_Abstract::makeNumbersVector(int n, int min, int max)
{
	const int bins = max - min + 1;
	QVector<int> result(bins);
	for (int i = 0; i < n; ++i) {
		int number = getNumber(min, max);
		if ((number < min) || (number > max))
			qDebug() << "getNumber(" << min << "," << max << ") returned " << number;
		else
			result[number - min]++;
	}
	return result;
}

double RNG_Abstract::testRandom(const QVector<int> &numbers) const
{
	int n = 0;
	for (int i = 0; i < numbers.size(); ++i)
		n += numbers[i];
	double expected = (double) n / (double) numbers.size();
	double chisq = 0;
	for (int i = 0; i < numbers.size(); ++i)
		chisq += ((double) numbers[i] - expected) * ((double) numbers[i] - expected) / expected;
	
	return chisq;
}
