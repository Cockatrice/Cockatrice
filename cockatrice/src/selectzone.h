#ifndef SELECTZONE_H
#define SELECTZONE_H

#include "cardzone.h"

class SelectZone : public CardZone {
	Q_OBJECT
private:
	QPointF selectionOrigin;
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
public:
	SelectZone(Player *_player, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent = 0, bool isView = false);
};

#endif
