#ifndef ZONECLOSEBUTTON_H
#define ZONECLOSEBUTTON_H

#include <QGraphicsWidget>

class ZoneCloseButton : public QGraphicsWidget {
	Q_OBJECT
signals:
	void triggered();
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	ZoneCloseButton(QGraphicsItem *parent = 0);
};

#endif
