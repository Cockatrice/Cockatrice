#ifndef RANDOM_H
#define RANDOM_H

#include <QObject>
#include <QDateTime>
#include <stdlib.h>

class Random : public QObject {
	Q_OBJECT
private:
	bool initialized;
public:
	Random(QObject *parent) : QObject(parent), initialized(false) { }
	void init();
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
