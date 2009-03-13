#include "player.h"
#include "client.h"
#include "handzone.h"
#include "tablezone.h"
#include "libraryzone.h"
#include "gravezone.h"
#include "rfgzone.h"
#include "sideboardzone.h"
#include <QMenu>

Player::Player(const QString &_name, int _id, QPointF _base, bool _local, CardDatabase *_db, Client *_client)
	: QObject(), defaultNumberTopCards(3), name(_name), id(_id), base(_base), local(_local), db(_db), client(_client)
{
	LibraryZone *deck = new LibraryZone(this);
	deck->setPos(_base);

	// XXX
	qreal foo = deck->boundingRect().height();

	GraveZone *grave = new GraveZone(this);
	grave->setPos(_base + QPointF(0, foo));

	RfgZone *rfg = new RfgZone(this);
	rfg->setPos(_base + QPointF(0, 2 * foo));

	SideboardZone *sb = new SideboardZone(this);
	sb->setPos(_base + QPointF(0, 3 * foo));

	Counter *life = new Counter(this, "life");
	life->setPos(_base + QPointF(-50, 5 * foo));

	_base += QPointF(deck->boundingRect().width(), 0);

	PlayerZone *hand = new HandZone(this);
	hand->setPos(_base);
	_base += QPointF(hand->boundingRect().width(), 0);

	PlayerZone *table = new TableZone(this);
	table->setPos(_base);

	aMoveHandToTopLibrary = new QAction(tr("Move to &top of library"), this);
	connect(aMoveHandToTopLibrary, SIGNAL(triggered()), this, SLOT(actMoveHandToTopLibrary()));
	aMoveHandToBottomLibrary = new QAction(tr("Move to &bottom of library"), this);
	connect(aMoveHandToBottomLibrary, SIGNAL(triggered()), this, SLOT(actMoveHandToBottomLibrary()));

	aViewLibrary = new QAction(tr("&View library"), this);
	if (local)
		aViewLibrary->setShortcut(tr("F5"));
	connect(aViewLibrary, SIGNAL(triggered()), this, SLOT(actViewLibrary()));
	aViewTopCards = new QAction(tr("View &top cards of library..."), this);
	connect(aViewTopCards, SIGNAL(triggered()), this, SLOT(actViewTopCards()));

	aViewGraveyard = new QAction(tr("&View graveyard"), this);
	if (local)
		aViewGraveyard->setShortcut(tr("F6"));
	connect(aViewGraveyard, SIGNAL(triggered()), this, SLOT(actViewGraveyard()));

	aViewRfg = new QAction(tr("&View removed cards"), this);
	connect(aViewRfg, SIGNAL(triggered()), this, SLOT(actViewRfg()));

	aViewSideboard = new QAction(tr("&View sideboard"), this);
	connect(aViewSideboard, SIGNAL(triggered()), this, SLOT(actViewSideboard()));

	playerMenu = new QMenu(tr("Player \"%1\"").arg(name));

	QMenu *handMenu = playerMenu->addMenu(tr("&Hand"));
	handMenu->addAction(aMoveHandToTopLibrary);
	handMenu->addAction(aMoveHandToBottomLibrary);
	hand->setMenu(handMenu);

	QMenu *libraryMenu = playerMenu->addMenu(tr("&Library"));
	libraryMenu->addAction(aViewLibrary);
	libraryMenu->addAction(aViewTopCards);
	deck->setMenu(libraryMenu);

	QMenu *graveMenu = playerMenu->addMenu(tr("&Graveyard"));
	graveMenu->addAction(aViewGraveyard);
	grave->setMenu(graveMenu);

	QMenu *rfgMenu = playerMenu->addMenu(tr("&Removed cards"));
	rfgMenu->addAction(aViewRfg);
	rfg->setMenu(rfgMenu);

	QMenu *sbMenu = playerMenu->addMenu(tr("&Sideboard"));
	sbMenu->addAction(aViewSideboard);
	sb->setMenu(sbMenu);
}

Player::~Player()
{
	qDebug("Player destructor");

	for (int i = 0; i < zones.size(); i++)
		delete zones.at(i);

	for (int i = 0; i < counters.size(); i++)
		delete counters.at(i);
}

void Player::actMoveHandToTopLibrary()
{
	PlayerZone *library = zones.findZone("deck");
	zones.findZone("hand")->moveAllToZone(library->getName(), 0);
}

void Player::actMoveHandToBottomLibrary()
{
	PlayerZone *library = zones.findZone("deck");
	zones.findZone("hand")->moveAllToZone(library->getName(), library->getCards()->size());
}

void Player::actViewLibrary()
{
	emit addZoneView(this, "deck", 0);
}

void Player::actViewTopCards()
{
	bool ok;
	int number = QInputDialog::getInteger(0, tr("View top cards of library"), tr("Number of cards:"), defaultNumberTopCards, 1, 2000000000, 1, &ok);
	if (ok) {
		defaultNumberTopCards = number;
		emit addZoneView(this, "deck", number);
	}
}

void Player::actViewGraveyard()
{
	emit addZoneView(this, "grave", 0);
}

void Player::actViewRfg()
{
	emit addZoneView(this, "rfg", 0);
}

void Player::actViewSideboard()
{
	emit addZoneView(this, "sb", 0);
}

void Player::addZone(PlayerZone *z)
{
	zones << z;
}

void Player::addCounter(Counter *c)
{
	counters << c;
}

void Player::gameEvent(ServerEventData *event)
{
	QStringList data = event->getEventData();
	switch (event->getEventType()) {
		case eventSetupZones: {
			// XXX Life counter
			int life = data[0].toInt();
			int deck_cards = data[1].toInt();
			int sb_cards = data[2].toInt();
			// XXX Fehlerbehandlung

			// Clean up existing zones first
			for (int i = 0; i < zones.size(); i++)
				zones.at(i)->clearContents();

			Counter *lifeCounter = counters.findCounter("life");
			lifeCounter->setValue(life);

			PlayerZone *deck = zones.findZone("deck");
			for (; deck_cards; deck_cards--)
				deck->addCard(new CardItem(db, QString(), -1));
			deck->reorganizeCards();

			PlayerZone *sb = zones.findZone("sb");
			for (; sb_cards; sb_cards--)
				sb->addCard(new CardItem(db, QString(), -1));
			sb->reorganizeCards();

			break;
		}
		case eventDraw: {
			PlayerZone *deck = zones.findZone("deck");
			PlayerZone *hand = zones.findZone("hand");
			if (!event->getPublic()) {
				hand->addCard(deck->takeCard(0, data[0].toInt(), data[1]));
			} else {
				int number = data[0].toInt();
				for (; number; number--)
					hand->addCard(deck->takeCard(0, -1, QString()));
			}
			break;
		}
		case eventMoveCard: {
			if (data.size() != 7) {
				qDebug("error");
				// XXX
			}
			int cardId = data[0].toInt();
			QString cardName = data[1];
			PlayerZone *startZone = zones.findZone(data[2]);
			if (!startZone)
				qDebug(QString("start zone invalid: %1").arg(data[2]).toLatin1());
			int position = data[3].toInt();
			PlayerZone *targetZone = zones.findZone(data[4]);
			if (!targetZone)
				qDebug(QString("target zone invalid: %1").arg(data[4]).toLatin1());
			int x = data[5].toInt();
			int y = data[6].toInt();
			// XXX Mehr Fehlerbehandlung

			CardItem *card = startZone->takeCard(position, cardId, cardName);
			if (!card) // XXX
				qDebug("null");

			card->deleteDragItem();

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
			PlayerZone *zone = zones.findZone(data[0]);
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
			PlayerZone *zone = zones.findZone(data[0]);
			int cardId = data[1].toInt();
			CardItem *card = zone->getCard(cardId, "");
			QString aname = data[2];
			QString avalue = data[3];
			// XXX Fehlerbehandlung

			if (aname == "tapped") {
				bool tapped = avalue == "1";
				emit logSetTapped(name, card->getName(), tapped);
				card->setTapped(tapped);
			} else if (aname == "attacking")
				card->setAttacking(avalue == "1");
			else if (aname == "facedown")
				card->setFacedown(avalue == "1");
			else if (aname == "counters") {
				int value = avalue.toInt();
				emit logSetCardCounters(name, card->getName(), value, card->getCounters());
				card->setCounters(value);
			} else if (aname == "annotation")
				card->setAnnotation(avalue);
			break;
		}
		case eventSetCounter: {
			if (data.size() != 2) {
				// XXX
			}
			int value = data[1].toInt();
			Counter *c = counters.findCounter(data[0]);
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
