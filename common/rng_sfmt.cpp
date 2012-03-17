#include "rng_sfmt.h"
#include "sfmt/SFMT.h"
#include <QDateTime>
#include <stdlib.h>
#include <iostream>

RNG_SFMT::RNG_SFMT(QObject *parent)
	: RNG_Abstract(parent)
{
	std::cerr << "Using SFMT random number generator." << std::endl;
	
	int seed = QDateTime::currentDateTime().toTime_t();
	init_gen_rand(seed);
	for (int i = 0; i < 100000; ++i)
		gen_rand64();
}

unsigned int RNG_SFMT::getNumber(unsigned int min, unsigned int max)
{
	mutex.lock();
	uint64_t r = gen_rand64();
	mutex.unlock();
	
	return min + (unsigned int) (((double) (max + 1 - min)) * r / (18446744073709551616.0 + 1.0));
}
