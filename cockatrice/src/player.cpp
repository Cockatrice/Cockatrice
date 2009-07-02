#include "player.h"
#include "client.h"
#include "cardzone.h"
#include "playerarea.h"
#include "counter.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "game.h"
#include <QGraphicsScene>
#include <QMenu>

Player::Player(const QString &_name, int _id, QPointF _base, bool _local, CardDatabase *_db, Client *_client, QGraphicsScene *_scene, Game *_parent)
	: QObject(_parent), defaultNumberTopCards(3), name(_name), id(_id), base(_base), local(_local), db(_db), client(_client)
{
	area = new PlayerArea(this);
	area->setPos(_base);
	_scene->addItem(area);

	aMoveHandToTopLibrary = new QAction(tr("Move to &top of library"), this);
	connect(aMoveHandToTopLibrary, SIGNAL(triggered()), this, SLOT(actMoveHandToTopLibrary()));
	aMoveHandToBottomLibrary = new QAction(tr("Move to &bottom of library"), this);
	connect(aMoveHandToBottomLibrary, SIGNAL(triggered()), this, SLOT(actMoveHandToBottomLibrary()));

	aViewLibrary = new QAction(tr("&View library"), this);
	if (local)
		aViewLibrary->setShortcut(tr("F3"));
	connect(aViewLibrary, SIGNAL(triggered()), this, SLOT(actViewLibrary()));
	aViewTopCards = new QAction(tr("View &top cards of library..."), this);
	connect(aViewTopCards, SIGNAL(triggered()), this, SLOT(actViewTopCards()));

	aViewGraveyard = new QAction(tr("&View graveyard"), this);
	if (local)
		aViewGraveyard->setShortcut(tr("F4"));
	connect(aViewGraveyard, SIGNAL(triggered()), this, SLOT(actViewGraveyard()));

	aViewRfg = new QAction(tr("&View removed cards"), this);
	connect(aViewRfg, SIGNAL(triggered()), this, SLOT(actViewRfg()));

	aViewSideboard = new QAction(tr("&View sideboard"), this);
	connect(aViewSideboard, SIGNAL(triggered()), this, SLOT(actViewSideboard()));

	playerMenu = new QMenu(tr("Player \"%1\"").arg(name));

	QMenu *handMenu = playerMenu->addMenu(tr("&Hand"));
	handMenu->addAction(aMoveHandToTopLibrary);
	handMenu->addAction(aMoveHandToBottomLibrary);
	zones.findZone("hand")->setMenu(handMenu);

	QMenu *libraryMenu = playerMenu->addMenu(tr("&Library"));
	libraryMenu->addAction(aViewLibrary);
	libraryMenu->addAction(aViewTopCards);
	zones.findZone("deck")->setMenu(libraryMenu, aViewLibrary);

	QMenu *graveMenu = playerMenu->addMenu(tr("&Graveyard"));
	graveMenu->addAction(aViewGraveyard);
	zones.findZone("grave")->setMenu(graveMenu, aViewGraveyard);

	QMenu *rfgMenu = playerMenu->addMenu(tr("&Removed cards"));
	rfgMenu->addAction(aViewRfg);
	zones.findZone("rfg")->setMenu(rfgMenu, aViewRfg);

	QMenu *sbMenu = playerMenu->addMenu(tr("&Sideboard"));
	sbMenu->addAction(aViewSideboard);
	zones.findZone("sb")->setMenu(sbMenu, aViewSideboard);
}

Player::~Player()
{
	qDebug("Player destructor");

	for (int i = 0; i < zones.size(); i++)
		delete zones.at(i);

	delete area;
}

void Player::actMoveHandToTopLibrary()
{
	zones.findZone("hand")->moveAllToZone("deck", 0);
}

void Player::actMoveHandToBottomLibrary()
{
	zones.findZone("hand")->moveAllToZone("deck", -1);
}

void Player::actViewLibrary()
{
	emit toggleZoneView(this, "deck", 0);
}

void Player::actViewTopCards()
{
	bool ok;
	int number = QInputDialog::getInteger(0, tr("View top cards of library"), tr("Number of cards:"), defaultNumberTopCards, 1, 2000000000, 1, &ok);
	if (ok) {
		defaultNumberTopCards = number;
		emit toggleZoneView(this, "deck", number);
	}
}

void Player::actViewGraveyard()
{
	emit toggleZoneView(this, "grave", 0);
}

void Player::actViewRfg()
{
	emit toggleZoneView(this, "rfg", 0);
}

void Player::actViewSideboard()
{
	emit toggleZoneView(this, "sb", 0);
}

void Player::addZone(CardZone *z)
{
	zones << z;
}

void Player::setCardAttrHelper(CardItem *card, const QString &aname, const QString &avalue, bool allCards)
{
	if (aname == "tapped") {
		bool tapped = avalue == "1";
		if (!(!tapped && card->getDoesntUntap() && allCards)) {
			if (!allCards)
				emit logSetTapped(name, card->getName(), tapped);
			card->setTapped(tapped);
		}
	} else if (aname == "attacking")
		card->setAttacking(avalue == "1");
	else if (aname == "facedown")
		card->setFaceDown(avalue == "1");
	else if (aname == "counters") {
		int value = avalue.toInt();
		emit logSetCardCounters(name, card->getName(), value, card->getCounters());
		card->setCounters(value);
	} else if (aname == "annotation")
		card->setAnnotation(avalue);
	else if (aname == "doesnt_untap") {
		bool value = (avalue == "1");
		emit logSetDoesntUntap(name, card->getName(), value);
		card->setDoesntUntap(value);
	}
}

void Player::gameEvent(const ServerEventData &event)
{
	QStringList data = event.getEventData();
	switch (event.getEventType()) {
		case eventSetupZones: {
			// XXX Life counter
			int deck_cards = data[0].toInt();
			int sb_cards = data[1].toInt();
			// XXX Fehlerbehandlung

			// Clean up existing zones first
			for (int i = 0; i < zones.size(); i++) {
				if (ZoneViewZone *view = zones.at(i)->getView())
					((ZoneViewWidget *) view->parentItem())->close();
				zones.at(i)->clearContents();
			}

			area->clearCounters();

			CardZone *deck = zones.findZone("deck");
			for (; deck_cards; deck_cards--)
				deck->addCard(new CardItem(db), false, -1);

			CardZone *sb = zones.findZone("sb");
			for (; sb_cards; sb_cards--)
				sb->addCard(new CardItem(db), false, -1);

			for (int i = 0; i < zones.size(); i++)
				zones.at(i)->reorganizeCards();

			if (local) {
				client->addCounter("life", Qt::white, 20);
				client->addCounter("w", QColor(255, 255, 150), 0);
				client->addCounter("u", QColor(150, 150, 255), 0);
				client->addCounter("b", QColor(150, 150, 150), 0);
				client->addCounter("r", QColor(250, 150, 150), 0);
				client->addCounter("g", QColor(150, 255, 150), 0);
				client->addCounter("x", QColor(255, 255, 255), 0);
				client->addCounter("storm", QColor(255, 255, 255), 0);
			}

			break;
		}
		case eventDraw: {
			CardZone *deck = zones.findZone("deck");
			CardZone *hand = zones.findZone("hand");
			if (!event.getPublic()) {
				hand->addCard(deck->takeCard(0, data[0].toInt(), data[1]), true, -1);
			} else {
				int number = data[0].toInt();
				for (; number; number--)
					hand->addCard(deck->takeCard(0, -1, QString()), false, -1);
				hand->reorganizeCards();
			}
			break;
		}
		case eventMoveCard: {
			if (data.size() != 8) {
				qDebug("error");
				// XXX
			}
			int cardId = data[0].toInt();
			QString cardName = data[1];
			CardZone *startZone = zones.findZone(data[2]);
			if (!startZone)
				qDebug(QString("start zone invalid: %1").arg(data[2]).toLatin1());
			int position = data[3].toInt();
			CardZone *targetZone = zones.findZone(data[4]);
			if (!targetZone)
				qDebug(QString("target zone invalid: %1").arg(data[4]).toLatin1());
			int x = data[5].toInt();
			int y = data[6].toInt();
			bool facedown = data[7].toInt();
			// XXX Mehr Fehlerbehandlung

			CardItem *card = startZone->takeCard(position, cardId, cardName);
			if (!card) // XXX
				qDebug("moveCard: card not found");

			card->deleteDragItem();

			card->setFaceDown(facedown);

			// The log event has to be sent before the card is added to the target zone
			// because the addCard function can modify the card object.
			emit logMoveCard(name, card->getName(), startZone->getName(), targetZone->getName());

			targetZone->addCard(card, true, x, y);

			break;
		}
		case eventCreateToken: {
			// zone, cardid, cardname, powtough, x, y
			if (data.size() != 6) {
				qDebug("error");
			}
			CardZone *zone = zones.findZone(data[0]);
			int cardid = data[1].toInt();
			QString cardname = data[2];
			QString powtough = data[3];
			int x = data[4].toInt();
			int y = data[5].toInt();

			CardItem *card = new CardItem(db, cardname, cardid);

			emit logCreateToken(name, card->getName());
			zone->addCard(card, true, x, y);

			break;
		}
		case eventSetCardAttr: {
			if (data.size() != 4) {
				// XXX
			}
			CardZone *zone = zones.findZone(data[0]);
			int cardId = data[1].toInt();
			QString aname = data[2];
			QString avalue = data[3];
			// XXX Fehlerbehandlung

			if (cardId == -1) {
				CardList *const cards = zone->getCards();
				for (int i = 0; i < cards->size(); i++)
					setCardAttrHelper(cards->at(i), aname, avalue, true);
				if (aname == "tapped")
					emit logSetTapped(name, QString("-1"), avalue == "1");
			} else {
				CardItem *card = zone->getCard(cardId, "");
				setCardAttrHelper(card, aname, avalue, false);
			}
			break;
		}
		case eventAddCounter: {
			if (data.size() != 3) {
				// XXX
			}
			QString counterName = data[0];
			int colorValue = data[1].toInt();
			int value = data[2].toInt();
			QColor color(colorValue / 65536, (colorValue % 65536) / 256, colorValue % 256);
			area->addCounter(counterName, color, value);
			break;
		}
		case eventSetCounter: {
			if (data.size() != 2) {
				// XXX
			}
			int value = data[1].toInt();
			QString counterName = data[0];
			Counter *c = area->getCounter(counterName);
			int oldValue = c->getValue();
			c->setValue(value);
			emit logSetCounter(name, c->getName(), value, oldValue);
			break;
		}
		default:
			qDebug("unhandled player event");
	}
}

void Player::hoverCardEvent(CardItem *card)
{
	emit hoverCard(card->getName());
}

void Player::showCardMenu(const QPoint &p)
{
	emit sigShowCardMenu(p);
}
