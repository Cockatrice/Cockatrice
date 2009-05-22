#ifndef RANDOM_H
#define RANDOM_H

#include <QObject>

class Random : public QObject {
	Q_OBJECT
public:
	Random(QObject *parent = 0);
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
