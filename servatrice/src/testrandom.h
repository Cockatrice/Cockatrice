#ifndef TESTRANDOM_H
#define TESTRANDOM_H

#include <QObject>
#include <QDateTime>
#include <stdlib.h>

class TestRandom : public QObject {
	Q_OBJECT
private:
	bool initialized;
public:
	TestRandom(QObject *parent) : QObject(parent), initialized(false) { }
	void init();
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
