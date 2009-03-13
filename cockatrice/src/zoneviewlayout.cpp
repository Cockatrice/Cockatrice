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
	resize(views.size() * 150, 1000);
	for (int i = 0; i < views.size(); i++) {
		views.at(i)->setPos(i * 150, 0);
	}
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
	delete item;
	reorganize();
}
