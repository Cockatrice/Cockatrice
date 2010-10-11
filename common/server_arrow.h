#ifndef SERVER_ARROW_H
#define SERVER_ARROW_H

#include "color.h"

class Server_Card;
class Server_ArrowTarget;

class Server_Arrow {
private:
	int id;
	Server_Card *startCard;
	Server_ArrowTarget *targetItem;
	Color color;
public:
	Server_Arrow(int _id, Server_Card *_startCard, Server_ArrowTarget *_targetItem, const Color &_color)
		: id(_id), startCard(_startCard), targetItem(_targetItem), color(_color) { }
	int getId() const { return id; }
	Server_Card *getStartCard() const { return startCard; }
	Server_ArrowTarget *getTargetItem() const { return targetItem; }
	const Color &getColor() const { return color; }
};

#endif
