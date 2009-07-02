#include "zoneviewlayout.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
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
//		qreal h = viewSize.bottom() - viewSize.top();
		views.at(i)->setPos(totalWidth, y);
		totalWidth += w;
	}
	resize(totalWidth, scene()->sceneRect().height());
	emit sizeChanged();
}

void ZoneViewLayout::toggleZoneView(Player *player, const QString &zoneName, int numberCards)
{
	for (int i = 0; i < views.size(); i++) {
		ZoneViewZone *temp = views[i]->getZone();
		if ((temp->getName() == zoneName) && (temp->getPlayer() == player)) { // view is already open
			removeItem(views[i]);
			if (temp->getNumberCards() == numberCards)
				return;
		}
	}

	ZoneViewWidget *item = new ZoneViewWidget(db, player, player->getZones()->findZone(zoneName), numberCards, this);
	views.append(item);
	connect(item, SIGNAL(closePressed(ZoneViewWidget *)), this, SLOT(removeItem(ZoneViewWidget *)));
	reorganize();
}

void ZoneViewLayout::removeItem(ZoneViewWidget *item)
{
	qDebug("ZoneViewLayout::removeItem");
	delete views.takeAt(views.indexOf(item));
	reorganize();
}

void ZoneViewLayout::removeItem(ZoneViewZone *item)
{
	removeItem(dynamic_cast<ZoneViewWidget *>(item->parentItem()));
}

void ZoneViewLayout::closeMostRecentZoneView()
{
	if (views.isEmpty())
		return;
	views.at(views.size() - 1)->close();
}

void ZoneViewLayout::clear()
{
	for (int i = views.size() - 1; i >= 0; i--)
		views.at(i)->close();
}
