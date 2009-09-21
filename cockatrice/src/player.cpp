#include "player.h"
#include "client.h"
#include "cardzone.h"
#include "counter.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "game.h"
#include "pilezone.h"
#include "tablezone.h"
#include "handzone.h"
#include <QSettings>
#include <QPainter>
#include <QMenu>

Player::Player(const QString &_name, int _id, bool _local, CardDatabase *_db, Client *_client, Game *_parent)
	: QObject(_parent), defaultNumberTopCards(3), name(_name), id(_id), active(false), local(_local), db(_db), client(_client)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/playerarea").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	setCacheMode(DeviceCoordinateCache);
	
	QPointF base = QPointF(55, 50);

	PileZone *deck = new PileZone(this, "deck", true, false, this);
	deck->setPos(base);

	qreal h = deck->boundingRect().height() + 20;

	PileZone *grave = new PileZone(this, "grave", false, true, this);
	grave->setPos(base + QPointF(0, h));

	PileZone *rfg = new PileZone(this, "rfg", false, true, this);
	rfg->setPos(base + QPointF(0, 2 * h));

	PileZone *sb = new PileZone(this, "sb", false, true, this);
	sb->setVisible(false);

	table = new TableZone(this, this);
	connect(table, SIGNAL(sizeChanged()), this, SLOT(updateBoundingRect()));
	hand = new HandZone(this, (int) table->boundingRect().height(), this);
	
	base = QPointF(deck->boundingRect().width() + 60, 0);
	hand->setPos(base);
	base += QPointF(hand->boundingRect().width(), 0);
	table->setPos(base);
	
	updateBoundingRect();

	if (local) {
		aMoveHandToTopLibrary = new QAction(this);
		connect(aMoveHandToTopLibrary, SIGNAL(triggered()), this, SLOT(actMoveHandToTopLibrary()));
		aMoveHandToBottomLibrary = new QAction(this);
		connect(aMoveHandToBottomLibrary, SIGNAL(triggered()), this, SLOT(actMoveHandToBottomLibrary()));

		aViewLibrary = new QAction(this);
		connect(aViewLibrary, SIGNAL(triggered()), this, SLOT(actViewLibrary()));
		aViewTopCards = new QAction(this);
		connect(aViewTopCards, SIGNAL(triggered()), this, SLOT(actViewTopCards()));
	}

	aViewGraveyard = new QAction(this);
	connect(aViewGraveyard, SIGNAL(triggered()), this, SLOT(actViewGraveyard()));

	aViewRfg = new QAction(this);
	connect(aViewRfg, SIGNAL(triggered()), this, SLOT(actViewRfg()));

	if (local) {
		aViewSideboard = new QAction(this);
		connect(aViewSideboard, SIGNAL(triggered()), this, SLOT(actViewSideboard()));
	
		aDrawCard = new QAction(this);
		connect(aDrawCard, SIGNAL(triggered()), this, SLOT(actDrawCard()));
		aDrawCards = new QAction(this);
		connect(aDrawCards, SIGNAL(triggered()), this, SLOT(actDrawCards()));
		aShuffle = new QAction(this);
		connect(aShuffle, SIGNAL(triggered()), this, SLOT(actShuffle()));
	}

	playerMenu = new QMenu(QString());

	if (local) {
		handMenu = playerMenu->addMenu(QString());
		handMenu->addAction(aMoveHandToTopLibrary);
		handMenu->addAction(aMoveHandToBottomLibrary);
		hand->setMenu(handMenu);

		libraryMenu = playerMenu->addMenu(QString());
		libraryMenu->addAction(aDrawCard);
		libraryMenu->addAction(aDrawCards);
		libraryMenu->addSeparator();
		libraryMenu->addAction(aShuffle);
		libraryMenu->addSeparator();
		libraryMenu->addAction(aViewLibrary);
		libraryMenu->addAction(aViewTopCards);
		deck->setMenu(libraryMenu, aDrawCard);
	} else {
		handMenu = 0;
		libraryMenu = 0;
	}

	graveMenu = playerMenu->addMenu(QString());
	graveMenu->addAction(aViewGraveyard);
	grave->setMenu(graveMenu, aViewGraveyard);

	rfgMenu = playerMenu->addMenu(QString());
	rfgMenu->addAction(aViewRfg);
	rfg->setMenu(rfgMenu, aViewRfg);

	if (local) {
		sbMenu = playerMenu->addMenu(QString());
		sbMenu->addAction(aViewSideboard);
		sb->setMenu(sbMenu, aViewSideboard);

		aUntapAll = new QAction(this);
		connect(aUntapAll, SIGNAL(triggered()), this, SLOT(actUntapAll()));

		aDecLife = new QAction(this);
		connect(aDecLife, SIGNAL(triggered()), this, SLOT(actDecLife()));
		aIncLife = new QAction(this);
		connect(aIncLife, SIGNAL(triggered()), this, SLOT(actIncLife()));
		aSetLife = new QAction(this);
		connect(aSetLife, SIGNAL(triggered()), this, SLOT(actSetLife()));
	
		aRollDie = new QAction(this);
		connect(aRollDie, SIGNAL(triggered()), this, SLOT(actRollDie()));
	
		aCreateToken = new QAction(this);
		connect(aCreateToken, SIGNAL(triggered()), this, SLOT(actCreateToken()));
		
		playerMenu->addSeparator();
		playerMenu->addAction(aUntapAll);
		playerMenu->addSeparator();
		playerMenu->addAction(aDecLife);
		playerMenu->addAction(aIncLife);
		playerMenu->addAction(aSetLife);
		playerMenu->addSeparator();
		playerMenu->addAction(aRollDie);
		playerMenu->addSeparator();
		playerMenu->addAction(aCreateToken);
		playerMenu->addSeparator();
		sayMenu = playerMenu->addMenu(QString());
		initSayMenu();

	} else
		sbMenu = 0;
	
	retranslateUi();
}

Player::~Player()
{
	qDebug("Player destructor");

	for (int i = 0; i < zones.size(); i++)
		delete zones.at(i);

	clearCounters();
	delete playerMenu;
}

void Player::updateBoundingRect()
{
	bRect = QRectF(0, 0, CARD_WIDTH + 60 + hand->boundingRect().width() + table->boundingRect().width(), table->boundingRect().height());
	emit sizeChanged();
}

void Player::retranslateUi()
{
	aViewGraveyard->setText(tr("&View graveyard"));
	aViewRfg->setText(tr("&View exile"));
	playerMenu->setTitle(tr("Player \"%1\"").arg(name));
	graveMenu->setTitle(tr("&Graveyard"));
	rfgMenu->setTitle(tr("&Exile"));
	
	if (local) {
		aMoveHandToTopLibrary->setText(tr("Move to &top of library"));
		aMoveHandToBottomLibrary->setText(tr("Move to &bottom of library"));
		aViewLibrary->setText(tr("&View library"));
		aViewLibrary->setShortcut(tr("F3"));
		aViewTopCards->setText(tr("View &top cards of library..."));
		aViewGraveyard->setShortcut(tr("F4"));
		aViewSideboard->setText(tr("&View sideboard"));
		aDrawCard->setText(tr("&Draw card"));
		aDrawCard->setShortcut(tr("Ctrl+D"));
		aDrawCards->setText(tr("D&raw cards..."));
		aDrawCards->setShortcut(tr("Ctrl+E"));
		aShuffle->setText(tr("&Shuffle"));
		aShuffle->setShortcut(tr("Ctrl+S"));
	
		handMenu->setTitle(tr("&Hand"));
		sbMenu->setTitle(tr("&Sideboard"));
		libraryMenu->setTitle(tr("&Library"));

		aUntapAll->setText(tr("&Untap all permanents"));
		aUntapAll->setShortcut(tr("Ctrl+U"));
		aDecLife->setText(tr("&Decrement life"));
		aDecLife->setShortcut(tr("F11"));
		aIncLife->setText(tr("&Increment life"));
		aIncLife->setShortcut(tr("F12"));
		aSetLife->setText(tr("&Set life"));
		aSetLife->setShortcut(tr("Ctrl+L"));
		aRollDie->setText(tr("R&oll die..."));
		aRollDie->setShortcut(tr("Ctrl+I"));
		aCreateToken->setText(tr("&Create token..."));
		aCreateToken->setShortcut(tr("Ctrl+T"));
		sayMenu->setTitle(tr("S&ay"));
	}
}

void Player::initSayMenu()
{
	sayMenu->clear();

	QSettings settings;
	settings.beginGroup("messages");
	int count = settings.value("count", 0).toInt();
	for (int i = 0; i < count; i++) {
		QAction *newAction = new QAction(settings.value(QString("msg%1").arg(i)).toString(), this);
		QString shortcut;
		switch (i) {
			case 0: shortcut = tr("F5"); break;
			case 1: shortcut = tr("F6"); break;
			case 2: shortcut = tr("F7"); break;
			case 3: shortcut = tr("F8"); break;
			case 4: shortcut = tr("F9"); break;
			case 5: shortcut = tr("F10"); break;
		}
		newAction->setShortcut(shortcut);
		connect(newAction, SIGNAL(triggered()), this, SLOT(actSayMessage()));
		sayMenu->addAction(newAction);
	}
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
	emit toggleZoneView(this, "deck", -1);
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
	emit toggleZoneView(this, "grave", -1);
}

void Player::actViewRfg()
{
	emit toggleZoneView(this, "rfg", -1);
}

void Player::actViewSideboard()
{
	emit toggleZoneView(this, "sb", -1);
}

void Player::actShuffle()
{
	client->shuffle();
}

void Player::actDrawCard()
{
	client->drawCards(1);
}

void Player::actDrawCards()
{
	int number = QInputDialog::getInteger(0, tr("Draw cards"), tr("Number:"));
	if (number)
		client->drawCards(number);
}

void Player::actUntapAll()
{
	client->setCardAttr("table", -1, "tapped", "false");
}

void Player::actIncLife()
{
	client->incCounter("life", 1);
}

void Player::actDecLife()
{
	client->incCounter("life", -1);
}

void Player::actSetLife()
{
	bool ok;
	int life = QInputDialog::getInteger(0, tr("Set life"), tr("New life total:"), getCounter("life")->getValue(), 0, 2000000000, 1, &ok);
	if (ok)
		client->setCounter("life", life);
}

void Player::actRollDie()
{
	bool ok;
	int sides = QInputDialog::getInteger(0, tr("Roll die"), tr("Number of sides:"), 20, 2, 1000, 1, &ok);
	if (ok)
		client->rollDie(sides);
}

void Player::actCreateToken()
{
	QString cardname = QInputDialog::getText(0, tr("Create token"), tr("Name:"));
	if (!cardname.isEmpty())
		client->createToken("table", cardname, QString(), 0, 0);
}

void Player::actSayMessage()
{
	QAction *a = qobject_cast<QAction *>(sender());
	client->say(a->text());
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
				emit logSetTapped(this, card->getName(), tapped);
			card->setTapped(tapped);
		}
	} else if (aname == "attacking")
		card->setAttacking(avalue == "1");
	else if (aname == "facedown")
		card->setFaceDown(avalue == "1");
	else if (aname == "counters") {
		int value = avalue.toInt();
		emit logSetCardCounters(this, card->getName(), value, card->getCounters());
		card->setCounters(value);
	} else if (aname == "annotation")
		card->setAnnotation(avalue);
	else if (aname == "doesnt_untap") {
		bool value = (avalue == "1");
		emit logSetDoesntUntap(this, card->getName(), value);
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

			clearCounters();

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

			int logPosition = position;
			int logX = x;
			if (position == -1)
				position = 0;
			if (x == -1)
				x = 0;
			CardItem *card = startZone->takeCard(position, cardId, cardName, startZone != targetZone);
			if (!card) // XXX
				qDebug("moveCard: card not found");

			card->deleteDragItem();

			card->setFaceDown(facedown);

			// The log event has to be sent before the card is added to the target zone
			// because the addCard function can modify the card object.
			emit logMoveCard(this, card->getName(), startZone, logPosition, targetZone, logX);

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

			emit logCreateToken(this, card->getName());
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
				const CardList &cards = zone->getCards();
				for (int i = 0; i < cards.size(); i++)
					setCardAttrHelper(cards.at(i), aname, avalue, true);
				if (aname == "tapped")
					emit logSetTapped(this, QString("-1"), avalue == "1");
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
			addCounter(counterName, color, value);
			break;
		}
		case eventSetCounter: {
			if (data.size() != 2) {
				// XXX
			}
			int value = data[1].toInt();
			QString counterName = data[0];
			Counter *c = getCounter(counterName);
			int oldValue = c->getValue();
			c->setValue(value);
			emit logSetCounter(this, c->getName(), value, oldValue);
			break;
		}
		default:
			qDebug("unhandled player event");
	}
}

void Player::showCardMenu(const QPoint &p)
{
	emit sigShowCardMenu(p);
}

void Player::setActive(bool _active)
{
	active = _active;
	update();
}

QRectF Player::boundingRect() const
{
	return bRect;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), QColor(200, 200, 200));
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));

	QString nameStr = getName();
	QFont font("Times");
	font.setPixelSize(20);
//	font.setWeight(QFont::Bold);
	
	int totalWidth = CARD_WIDTH + 60;
	
	if (getActive()) {
		QFontMetrics fm(font);
		double w = fm.width(nameStr) * 1.7;
		double h = fm.height() * 1.7;
		if (w < h)
			w = h;
		
		painter->setPen(Qt::transparent);
		QRadialGradient grad(QPointF(0.5, 0.5), 0.5);
		grad.setCoordinateMode(QGradient::ObjectBoundingMode);
		grad.setColorAt(0, QColor(150, 200, 150, 255));
		grad.setColorAt(0.7, QColor(150, 200, 150, 255));
		grad.setColorAt(1, QColor(150, 150, 150, 0));
		painter->setBrush(QBrush(grad));
		
		painter->drawEllipse(QRectF(((double) (totalWidth - w)) / 2, 0, w, h));
	}
	painter->setFont(font);
	painter->setPen(QPen(Qt::black));
	painter->drawText(QRectF(0, 0, totalWidth, 40), Qt::AlignCenter, nameStr);
}

Counter *Player::getCounter(const QString &name, bool remove)
{
	for (int i = 0; i < counterList.size(); i++) {
		Counter *temp = counterList.at(i);
		if (temp->getName() == name) {
			if (remove)
				counterList.removeAt(i);
			return temp;
		}
	}
	return 0;
}

void Player::addCounter(const QString &name, QColor color, int value)
{
	counterList.append(new Counter(this, name, color, value, this));
	rearrangeCounters();
}

void Player::delCounter(const QString &name)
{
	delete getCounter(name, true);
	rearrangeCounters();
}

void Player::clearCounters()
{
	for (int i = 0; i < counterList.size(); i++)
		delete counterList.at(i);
	counterList.clear();
}

void Player::rearrangeCounters()
{
	const int counterAreaWidth = 55;
	qreal y = 50;
	for (int i = 0; i < counterList.size(); i++) {
		Counter *temp = counterList.at(i);
		QRectF br = temp->boundingRect();
		temp->setPos((counterAreaWidth - br.width()) / 2, y);
		y += br.height() + 10;
	}
}
