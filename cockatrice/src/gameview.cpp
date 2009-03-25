#include "gameview.h"

GameView::GameView(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setRenderHints(QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
//	setOptimizationFlags(/*DontClipPainter | */DontSavePainterState);
	setDragMode(RubberBandDrag);
}

void GameView::scaleToScene()
{
	// This function ensures that the bounding rectangles of card pictures
	// have integer sizes. This is achieved by using a scale factor of n / 6.
	QRectF sr = scene()->sceneRect();
	QRectF vr = QRectF(viewport()->rect().adjusted(2, 2, -2, -2));
	qreal scale_h = vr.width() / sr.width();
	qreal scale_v = vr.height() / sr.height();
	qreal scale_total = qMin(scale_h, scale_v);
	
	qreal scale_corr = (qreal) ((int) (scale_total * 6)) / 6;
	
	qDebug(QString("scale_corr = %1 / 6").arg(scale_corr * 6).toLatin1());
	setTransform(QTransform().scale(scale_corr, scale_corr));
}

void GameView::resizeEvent(QResizeEvent *event)
{
	qDebug("GameView: resizeEvent");
	QGraphicsView::resizeEvent(event);
	scaleToScene();
}
