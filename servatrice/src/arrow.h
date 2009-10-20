#ifndef ARROW_H
#define ARROW_H

class Card;

class Arrow {
private:
	Card *startCard, *targetCard;
	int color;
public:
	Arrow(Card *_startCard, Card *_targetCard, int _color)
		: startCard(_startCard), targetCard(_targetCard), color(_color) { }
	Card *getStartCard() const { return startCard; }
	Card *getTargetCard() const { return targetCard; }
	int getColor() const { return color; }
};

#endif