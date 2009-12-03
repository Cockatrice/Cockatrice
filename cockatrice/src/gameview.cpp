#include "gameview.h"

GameView::GameView(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
	setDragMode(RubberBandDrag);
	setViewportUpdateMode(BoundingRectViewportUpdate);

	connect(scene, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(updateSceneRect(const QRectF &)));
}

void GameView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	updateSceneRect(scene()->sceneRect());
}

void GameView::updateSceneRect(const QRectF &rect)
{
	qDebug(QString("updateSceneRect = %1,%2").arg(rect.width()).arg(rect.height()).toLatin1());
	fitInView(rect, Qt::KeepAspectRatio);
}
