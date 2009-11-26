#ifndef SERVER_ARROW_H
#define SERVER_ARROW_H

#include <QColor>

class Server_Card;

class Server_Arrow {
private:
	int id;
	Server_Card *startCard, *targetCard;
	QColor color;
public:
	Server_Arrow(int _id, Server_Card *_startCard, Server_Card *_targetCard, const QColor &_color)
		: id(_id), startCard(_startCard), targetCard(_targetCard), color(_color) { }
	int getId() const { return id; }
	Server_Card *getStartCard() const { return startCard; }
	Server_Card *getTargetCard() const { return targetCard; }
	QColor getColor() const { return color; }
};

#endif
