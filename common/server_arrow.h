#ifndef SERVER_ARROW_H
#define SERVER_ARROW_H

#include "pb/color.pb.h"

class Server_Card;
class Server_ArrowTarget;

class Server_Arrow {
private:
	int id;
	Server_Card *startCard;
	Server_ArrowTarget *targetItem;
	color arrowColor;
public:
	Server_Arrow(int _id, Server_Card *_startCard, Server_ArrowTarget *_targetItem, const color &_arrowColor)
		: id(_id), startCard(_startCard), targetItem(_targetItem), arrowColor(_arrowColor) { }
	int getId() const { return id; }
	Server_Card *getStartCard() const { return startCard; }
	Server_ArrowTarget *getTargetItem() const { return targetItem; }
	const color &getColor() const { return arrowColor; }
};

#endif
