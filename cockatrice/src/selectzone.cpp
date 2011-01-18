#include <QGraphicsSceneMouseEvent>
#include "selectzone.h"
#include "gamescene.h"

SelectZone::SelectZone(Player *_player, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent, bool isView)
	: CardZone(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent, isView)
{
}

void SelectZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::LeftButton)) {
		QPointF pos = event->pos();
		if (pos.x() < 0)
			pos.setX(0);
		QRectF br = boundingRect();
		if (pos.x() > br.width())
			pos.setX(br.width());
		if (pos.y() < 0)
			pos.setY(0);
		if (pos.y() > br.height())
			pos.setY(br.height());
		
		QRectF selectionRect = QRectF(selectionOrigin, pos).normalized();
		for (int i = 0; i < cards.size(); ++i)
			cards[i]->setSelected(selectionRect.intersects(cards[i]->mapRectToParent(cards[i]->boundingRect())));
		
		static_cast<GameScene *>(scene())->resizeRubberBand(scenePos() + pos);
		event->accept();
	}
}

void SelectZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		scene()->clearSelection();
		
		selectionOrigin = event->pos();
		static_cast<GameScene *>(scene())->startRubberBand(event->scenePos());
		event->accept();
	} else
		CardZone::mousePressEvent(event);
}

void SelectZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	selectionOrigin = QPoint();
	static_cast<GameScene *>(scene())->stopRubberBand();
	event->accept();
}
 
