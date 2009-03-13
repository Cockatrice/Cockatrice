#include <QtGui>

#include "zoneviewwidget.h"
#include "carditem.h"
#include "zoneviewzone.h"
#include "player.h"
#include "client.h"
#include "zoneclosebutton.h"

ZoneViewWidget::ZoneViewWidget(CardDatabase *_db, Player *_player, PlayerZone *_origZone, int numberCards, QGraphicsItem *parent)
	: QGraphicsWidget(parent), db(_db), player(_player)
{
	ZoneCloseButton *closeButton = new ZoneCloseButton(this);
	connect(closeButton, SIGNAL(triggered()), this, SLOT(slotClosePressed()));
	
	resize(150, 1000);
	
	zone = new ZoneViewZone(player, _origZone, numberCards, this);
	zone->setPos(0, 30);
	if (!zone->initializeCards()) {
		connect(player->client, SIGNAL(zoneDumpReceived(int, QList<ServerZoneCard *>)), this, SLOT(zoneDumpReceived(int, QList<ServerZoneCard *>)));
		cmdId = player->client->dumpZone(player->getId(), _origZone->getName(), numberCards);
	}
}

void ZoneViewWidget::zoneDumpReceived(int commandId, QList<ServerZoneCard *> cards)
{
	if (commandId != cmdId)
		return;
	
	for (int i = 0; i < cards.size(); i++) {
		ServerZoneCard *temp = cards[i];
		
		CardItem *card = new CardItem(db, temp->getName(), i, zone);
		zone->addCard(card, false, i);
		
		delete temp;
	}
	zone->reorganizeCards();
}

void ZoneViewWidget::slotClosePressed()
{
	emit closePressed(this);
}
