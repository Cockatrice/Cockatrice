#include <QtGui>

#include "zoneviewwidget.h"
#include "carditem.h"
#include "zoneviewzone.h"
#include "player.h"
#include "client.h"

ZoneViewWidget::ZoneViewWidget(CardDatabase *_db, Player *_player, CardZone *_origZone, int numberCards, QGraphicsItem *parent)
	: QGraphicsWidget(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint/* | Qt::WindowCloseButtonHint*/), db(_db), player(_player)
{
	setWindowTitle(QString("%1's %2").arg(player->getName()).arg(_origZone->getName()));
	setAttribute(Qt::WA_DeleteOnClose);

	qreal y = 10;
	if (_origZone->getIsShufflable() && (numberCards == 0)) {
		shuffleCheckBox = new QCheckBox("shuffle when closing");
		shuffleCheckBox->setChecked(true);
		QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget(this);
		shuffleProxy->setWidget(shuffleCheckBox);
		y += shuffleProxy->y() + shuffleProxy->size().height();
	} else
		shuffleCheckBox = 0;

	qreal left, top, right, bottom;
	getWindowFrameMargins(&left, &top, &right, &bottom);
	qreal h = scene()->sceneRect().height() - (top + bottom);

	scrollBar = new QScrollBar(Qt::Vertical);
	QGraphicsProxyWidget *scrollProxy = new QGraphicsProxyWidget(this);
	scrollProxy->setWidget(scrollBar);
	scrollProxy->setPos(138, y);
	scrollProxy->resize(scrollProxy->size().width(), h - y);

	qreal w = 138 + scrollProxy->size().width();
	resize(w, h);
	setMinimumSize(w, h);
	setMaximumSize(w, h);

	zone = new ZoneViewZone(player, _origZone, numberCards, this);
	zone->setPos(3, y);
	zone->setHeight((int) (h - y));
	if (!zone->initializeCards()) {
		connect(player->client, SIGNAL(zoneDumpReceived(int, QList<ServerZoneCard *>)), this, SLOT(zoneDumpReceived(int, QList<ServerZoneCard *>)));
		PendingCommand *dumpZoneCommand = player->client->dumpZone(player->getId(), _origZone->getName(), numberCards);
		cmdId = dumpZoneCommand->getMsgId();
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

void ZoneViewWidget::closeEvent(QCloseEvent *event)
{
	if (shuffleCheckBox)
		if (shuffleCheckBox->isChecked())
			player->client->shuffle();
	emit closePressed(this);
	deleteLater();
	event->accept();
}
