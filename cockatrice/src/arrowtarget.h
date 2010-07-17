#ifndef ARROWTARGET_H
#define ARROWTARGET_H

#include "abstractgraphicsitem.h"
#include <QList>

class Player;
class ArrowItem;

class ArrowTarget : public AbstractGraphicsItem {
protected:
	Player *owner;
private:
	bool beingPointedAt;
	QList<ArrowItem *> arrowsFrom, arrowsTo;
public:
	ArrowTarget(Player *_owner, QGraphicsItem *parent = 0);
	~ArrowTarget();
	
	Player *getOwner() const { return owner; }
	
	void setBeingPointedAt(bool _beingPointedAt);
	bool getBeingPointedAt() const { return beingPointedAt; }
	
	const QList<ArrowItem *> &getArrowsFrom() const { return arrowsFrom; }
	void addArrowFrom(ArrowItem *arrow) { arrowsFrom.append(arrow); }
	void removeArrowFrom(ArrowItem *arrow) { arrowsFrom.removeAt(arrowsFrom.indexOf(arrow)); }
	
	const QList<ArrowItem *> &getArrowsTo() const { return arrowsTo; }
	void addArrowTo(ArrowItem *arrow) { arrowsTo.append(arrow); }
	void removeArrowTo(ArrowItem *arrow) { arrowsTo.removeAt(arrowsTo.indexOf(arrow)); }
};

#endif