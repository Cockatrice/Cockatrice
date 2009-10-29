#ifndef RNG_ABSTRACT_H
#define RNG_ABSTRACT_H

#include <QObject>

class RNG_Abstract : public QObject {
	Q_OBJECT
public:
	RNG_Abstract(QObject *parent = 0) : QObject(parent) { }
	virtual unsigned int getNumber(unsigned int min, unsigned int max) = 0;
};

extern RNG_Abstract *rng;

#endif
