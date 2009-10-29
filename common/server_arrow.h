#ifndef SERVER_ARROW_H
#define SERVER_ARROW_H

class Server_Card;

class Server_Arrow {
private:
	int id;
	Server_Card *startCard, *targetCard;
	int color;
public:
	Server_Arrow(int _id, Server_Card *_startCard, Server_Card *_targetCard, int _color)
		: id(_id), startCard(_startCard), targetCard(_targetCard), color(_color) { }
	int getId() const { return id; }
	Server_Card *getStartCard() const { return startCard; }
	Server_Card *getTargetCard() const { return targetCard; }
	int getColor() const { return color; }
};

#endif