#ifndef ABSTRACTRNG_H
#define ABSTRACTRNG_H

#include <QObject>

class AbstractRNG : public QObject {
	Q_OBJECT
public:
	AbstractRNG(QObject *parent = 0) : QObject(parent) { }
	virtual unsigned int getNumber(unsigned int min, unsigned int max) = 0;
};

#endif
