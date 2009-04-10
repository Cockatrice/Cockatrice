#include "zoneviewlayout.h"
#include "zoneviewwidget.h"
#include "player.h"

ZoneViewLayout::ZoneViewLayout(CardDatabase *_db, QGraphicsItem *parent)
	: QGraphicsWidget(parent), db(_db)
{

}

void ZoneViewLayout::reorganize()
{
	qDebug(QString("ZoneViewLayout: activate: count=%1").arg(views.size()).toLatin1());
	if (views.isEmpty()) {
		resize(0, 0);
		emit sizeChanged();
		return;
	}
	qreal x, y;
	views.at(0)->getWindowFrameMargins(&x, &y, 0, 0);
	qreal totalWidth = x;
	for (int i = 0; i < views.size(); i++) {
		QRectF viewSize = views.at(i)->windowFrameRect();
		qreal w = viewSize.right() - viewSize.left();
		qreal h = viewSize.bottom() - viewSize.top();
		views.at(i)->setPos(totalWidth, y);
		totalWidth += w;
	}
	resize(totalWidth, scene()->sceneRect().height());
	emit sizeChanged();
}

void ZoneViewLayout::addItem(Player *player, const QString &zoneName, int numberCards)
{
	ZoneViewWidget *item = new ZoneViewWidget(db, player, player->getZones()->findZone(zoneName), numberCards, this);
	views.append(item);
	connect(item, SIGNAL(closePressed(ZoneViewWidget *)), this, SLOT(removeItem(ZoneViewWidget *)));
	reorganize();
}

void ZoneViewLayout::removeItem(ZoneViewWidget *item)
{
	qDebug("ZoneViewLayout::removeItem");
	views.removeAt(views.indexOf(item));
	scene()->removeItem(item);
	reorganize();
}

void ZoneViewLayout::closeMostRecentZoneView()
{
	if (views.isEmpty())
		return;
	views.at(views.size() - 1)->close();
}
