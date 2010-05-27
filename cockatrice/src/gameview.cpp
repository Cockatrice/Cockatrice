#include "gameview.h"
#include "gamescene.h"
#include <QResizeEvent>
#include <QAction>

GameView::GameView(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
	setDragMode(RubberBandDrag);
	setFocusPolicy(Qt::NoFocus);
	setViewportUpdateMode(BoundingRectViewportUpdate);

	connect(scene, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(updateSceneRect(const QRectF &)));

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
