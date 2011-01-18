#include "gameview.h"
#include "gamescene.h"
#include <QResizeEvent>
#include <QAction>
#include <QRubberBand>

GameView::GameView(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent), rubberBand(0)
{
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
	setFocusPolicy(Qt::NoFocus);
	setViewportUpdateMode(BoundingRectViewportUpdate);

	connect(scene, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(updateSceneRect(const QRectF &)));
	
	connect(scene, SIGNAL(sigStartRubberBand(const QPointF &)), this, SLOT(startRubberBand(const QPointF &)));
	connect(scene, SIGNAL(sigResizeRubberBand(const QPointF &)), this, SLOT(resizeRubberBand(const QPointF &)));
	connect(scene, SIGNAL(sigStopRubberBand()), this, SLOT(stopRubberBand()));

	aCloseMostRecentZoneView = new QAction(this);
	aCloseMostRecentZoneView->setShortcut(tr("Esc"));
	connect(aCloseMostRecentZoneView, SIGNAL(triggered()), scene, SLOT(closeMostRecentZoneView()));
	addAction(aCloseMostRecentZoneView);
}

void GameView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	GameScene *s = dynamic_cast<GameScene *>(scene());
	if (s) {
		s->processViewSizeChange(event->size());
	}
	updateSceneRect(scene()->sceneRect());
}

void GameView::updateSceneRect(const QRectF &rect)
{
	qDebug(QString("updateSceneRect = %1,%2").arg(rect.width()).arg(rect.height()).toLatin1());
	fitInView(rect, Qt::KeepAspectRatio);
}

void GameView::startRubberBand(const QPointF &_selectionOrigin)
{
	selectionOrigin = _selectionOrigin;
	rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	rubberBand->setGeometry(QRect(mapFromScene(selectionOrigin), QSize(0, 0)));
	rubberBand->show();
}

void GameView::resizeRubberBand(const QPointF &cursorPoint)
{
	if (rubberBand)
		rubberBand->setGeometry(QRect(mapFromScene(selectionOrigin), mapFromScene(cursorPoint)).normalized());
}

void GameView::stopRubberBand()
{
	if (rubberBand) {
		rubberBand->deleteLater();
		rubberBand = 0;
	}
}
