#ifndef ARROW_H
#define ARROW_H

class Card;

class Arrow {
private:
	int id;
	Card *startCard, *targetCard;
	int color;
public:
	Arrow(int _id, Card *_startCard, Card *_targetCard, int _color)
		: id(_id), startCard(_startCard), targetCard(_targetCard), color(_color) { }
	int getId() const { return id; }
	Card *getStartCard() const { return startCard; }
	Card *getTargetCard() const { return targetCard; }
	int getColor() const { return color; }
};

#endif