#ifndef SERVERZONECARD_H
#define SERVERZONECARD_H

#include <QString>

class ServerZoneCard {
private:
	int id;
	QString name;
	int x, y;
	int counters;
	bool tapped;
	bool attacking;
	QString annotation;
public:
	ServerZoneCard(int _id, const QString &_name, int _x, int _y, int _counters, bool _attacking, const QString &_annotation)
		: id(_id), name(_name), x(_x), y(_y), counters(_counters), attacking(_attacking), annotation(_annotation) { }
	int getId() const { return id; }
	QString getName() const { return name; }
	int getX() const { return x; }
	int getY() const { return y; }
	int getCounters() const { return counters; }
	bool getTapped() const { return tapped; }
	bool getAttacking() const { return attacking; }
	QString getAnnotation() const { return annotation; }
};

#endif
