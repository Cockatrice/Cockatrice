#include "gameview.h"

GameView::GameView(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
//	setOptimizationFlags(/*DontClipPainter | */DontSavePainterState);
	setDragMode(RubberBandDrag);
//	setViewportUpdateMode(FullViewportUpdate);
}

void GameView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}
