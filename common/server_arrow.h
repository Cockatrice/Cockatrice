#ifndef SERVER_ARROW_H
#define SERVER_ARROW_H

#include <QColor>

class Server_Card;
class Server_ArrowTarget;

class Server_Arrow {
private:
	int id;
	Server_Card *startCard;
	Server_ArrowTarget *targetItem;
	QColor color;
public:
	Server_Arrow(int _id, Server_Card *_startCard, Server_ArrowTarget *_targetItem, const QColor &_color)
		: id(_id), startCard(_startCard), targetItem(_targetItem), color(_color) { }
	int getId() const { return id; }
	Server_Card *getStartCard() const { return startCard; }
	Server_ArrowTarget *getTargetItem() const { return targetItem; }
	QColor getColor() const { return color; }
};

#endif
