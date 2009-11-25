#include "player.h"
#include "client.h"
#include "cardzone.h"
#include "counter.h"
#include "arrowitem.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "pilezone.h"
#include "tablezone.h"
#include "handzone.h"
#include "cardlist.h"
#include "tab_game.h"
#include <QSettings>
#include <QPainter>
#include <QMenu>

Player::Player(const QString &_name, int _id, bool _local, Client *_client, TabGame *_parent)
	: QObject(_parent), defaultNumberTopCards(3), name(_name), id(_id), active(false), local(_local), client(_client)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/playerarea").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	setCacheMode(DeviceCoordinateCache);
	
	QPointF base = QPointF(counterAreaWidth, 50);

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
	
	base = QPointF(deck->boundingRect().width() + counterAreaWidth + 5, 0);
	hand->setPos(base);
	base += QPointF(hand->boundingRect().width(), 0);
	table->setPos(base);
	
	updateBoundingRect();

	if (local) {
		aMoveHandToTopLibrary = new QAction(this);
		aMoveHandToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
		aMoveHandToBottomLibrary = new QAction(this);
		aMoveHandToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
		aMoveHandToGrave = new QAction(this);
		aMoveHandToGrave->setData(QList<QVariant>() << "grave" << 0);
		aMoveHandToRfg = new QAction(this);
		aMoveHandToRfg->setData(QList<QVariant>() << "rfg" << 0);
		
		connect(aMoveHandToTopLibrary, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));
		connect(aMoveHandToBottomLibrary, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));
		connect(aMoveHandToGrave, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));
		connect(aMoveHandToRfg, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));

		aMoveGraveToTopLibrary = new QAction(this);
		aMoveGraveToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
		aMoveGraveToBottomLibrary = new QAction(this);
		aMoveGraveToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
		aMoveGraveToHand = new QAction(this);
		aMoveGraveToHand->setData(QList<QVariant>() << "hand" << 0);
		aMoveGraveToRfg = new QAction(this);
		aMoveGraveToRfg->setData(QList<QVariant>() << "rfg" << 0);
		
		connect(aMoveGraveToTopLibrary, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));
		connect(aMoveGraveToBottomLibrary, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));
		connect(aMoveGraveToHand, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));
		connect(aMoveGraveToRfg, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));

		aMoveRfgToTopLibrary = new QAction(this);
		aMoveRfgToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
		aMoveRfgToBottomLibrary = new QAction(this);
		aMoveRfgToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
		aMoveRfgToHand = new QAction(this);
		aMoveRfgToHand->setData(QList<QVariant>() << "hand" << 0);
		aMoveRfgToGrave = new QAction(this);
		aMoveRfgToGrave->setData(QList<QVariant>() << "grave" << 0);
		
		connect(aMoveRfgToTopLibrary, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));
		connect(aMoveRfgToBottomLibrary, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));
		connect(aMoveRfgToHand, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));
		connect(aMoveRfgToGrave, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));

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
                aMulligan = new QAction(this);
                connect(aMulligan, SIGNAL(triggered()), this, SLOT(actMulligan()));
	}

	playerMenu = new QMenu(QString());

	if (local) {
		handMenu = playerMenu->addMenu(QString());
                handMenu->addAction(aMulligan);
		handMenu->addAction(aMoveHandToTopLibrary);
		handMenu->addAction(aMoveHandToBottomLibrary);
		handMenu->addAction(aMoveHandToGrave);
		handMenu->addAction(aMoveHandToRfg);
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
		graveMenu->addSeparator();
		graveMenu->addAction(aMoveGraveToTopLibrary);
		graveMenu->addAction(aMoveGraveToBottomLibrary);
		graveMenu->addAction(aMoveGraveToHand);
		graveMenu->addAction(aMoveGraveToRfg);

		rfgMenu->addSeparator();
		rfgMenu->addAction(aMoveRfgToTopLibrary);
		rfgMenu->addAction(aMoveRfgToBottomLibrary);
		rfgMenu->addAction(aMoveRfgToHand);
		rfgMenu->addAction(aMoveRfgToGrave);

		sbMenu = playerMenu->addMenu(QString());
		sbMenu->addAction(aViewSideboard);
		sb->setMenu(sbMenu, aViewSideboard);

		aUntapAll = new QAction(this);
		connect(aUntapAll, SIGNAL(triggered()), this, SLOT(actUntapAll()));

		aRollDie = new QAction(this);
		connect(aRollDie, SIGNAL(triggered()), this, SLOT(actRollDie()));
	
		aCreateToken = new QAction(this);
		connect(aCreateToken, SIGNAL(triggered()), this, SLOT(actCreateToken()));
		
		playerMenu->addSeparator();
		countersMenu = playerMenu->addMenu(QString());
		playerMenu->addSeparator();
		playerMenu->addAction(aUntapAll);
		playerMenu->addSeparator();
		playerMenu->addAction(aRollDie);
		playerMenu->addSeparator();
		playerMenu->addAction(aCreateToken);
		playerMenu->addSeparator();
		sayMenu = playerMenu->addMenu(QString());
		initSayMenu();
	} else {
		countersMenu = 0;
		sbMenu = 0;
	}
	
	retranslateUi();
}

Player::~Player()
{
	qDebug("Player destructor");

	QMapIterator<QString, CardZone *> i(zones);
	while (i.hasNext())
		delete i.next().value();

	clearCounters();
	clearArrows();
	delete playerMenu;
}

void Player::updateBoundingRect()
{
	// XXX! PREPARE GEOMETRY CHANGE
	bRect = QRectF(0, 0, CARD_WIDTH + 5 + counterAreaWidth + hand->boundingRect().width() + table->boundingRect().width(), table->boundingRect().height());
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
		aMoveHandToGrave->setText(tr("Move to g&raveyard"));
		aMoveHandToRfg->setText(tr("Move to &exile"));
		aMoveGraveToTopLibrary->setText(tr("Move to &top of library"));
		aMoveGraveToBottomLibrary->setText(tr("Move to &bottom of library"));
		aMoveGraveToHand->setText(tr("Move to &hand"));
		aMoveGraveToRfg->setText(tr("Move to &exile"));
		aMoveRfgToTopLibrary->setText(tr("Move to &top of library"));
		aMoveRfgToBottomLibrary->setText(tr("Move to &bottom of library"));
		aMoveRfgToHand->setText(tr("Move to &hand"));
		aMoveRfgToGrave->setText(tr("Move to g&raveyard"));
		aViewLibrary->setText(tr("&View library"));
		aViewLibrary->setShortcut(tr("F3"));
		aViewTopCards->setText(tr("View &top cards of library..."));
		aViewGraveyard->setShortcut(tr("F4"));
		aViewSideboard->setText(tr("&View sideboard"));
		aDrawCard->setText(tr("&Draw card"));
		aDrawCard->setShortcut(tr("Ctrl+D"));
		aDrawCards->setText(tr("D&raw cards..."));
		aDrawCards->setShortcut(tr("Ctrl+E"));
                aMulligan->setText(tr("Take &mulligan"));
                aMulligan->setShortcut(tr("Ctrl+M"));
		aShuffle->setText(tr("&Shuffle"));
		aShuffle->setShortcut(tr("Ctrl+S"));
	
		handMenu->setTitle(tr("&Hand"));
		sbMenu->setTitle(tr("&Sideboard"));
		libraryMenu->setTitle(tr("&Library"));
		countersMenu->setTitle(tr("&Counters"));

		aUntapAll->setText(tr("&Untap all permanents"));
		aUntapAll->setShortcut(tr("Ctrl+U"));
		aRollDie->setText(tr("R&oll die..."));
		aRollDie->setShortcut(tr("Ctrl+I"));
		aCreateToken->setText(tr("&Create token..."));
		aCreateToken->setShortcut(tr("Ctrl+T"));
		sayMenu->setTitle(tr("S&ay"));
		
		QMapIterator<int, Counter *> counterIterator(counters);
		while (counterIterator.hasNext())
			counterIterator.next().value()->retranslateUi();
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

void Player::actMulligan()
{
	if (mulliganCards <= 0)
		return;
	
	const CardList &handCards = hand->getCards();
	for (int i = 0; i < handCards.size(); i++)
		client->moveCard(handCards.at(i)->getId(), "hand", "deck", 0);
	client->shuffle();
	client->drawCards(mulliganCards--);
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
	zones.insert(z->getName(), z);
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

/*void Player::gameEvent(const ServerEventData &event)
{
	QStringList data = event.getEventData();
	switch (event.getEventType()) {
		case eventSetupZones: {
			// XXX Life counter
			int deck_cards = data[0].toInt();
			int sb_cards = data[1].toInt();
			// XXX Fehlerbehandlung

			// Clean up existing zones first
			QMapIterator<QString, CardZone *> zoneIterator(zones);
			while (zoneIterator.hasNext()) {
				zoneIterator.next();
				if (ZoneViewZone *view = zoneIterator.value()->getView())
					((ZoneViewWidget *) view->parentItem())->close();
				zoneIterator.value()->clearContents();
			}

			clearCounters();
			clearArrows();

			CardZone *deck = zones.value("deck");
			for (; deck_cards; deck_cards--)
				deck->addCard(new CardItem(db), false, -1);

			CardZone *sb = zones.value("sb");
			for (; sb_cards; sb_cards--)
				sb->addCard(new CardItem(db), false, -1);

			zoneIterator.toFront();
			while (zoneIterator.hasNext())
				zoneIterator.next().value()->reorganizeCards();

			if (local) {
				client->addCounter("life", Qt::white, 25, 20);
				client->addCounter("w", QColor(255, 255, 150), 20, 0);
				client->addCounter("u", QColor(150, 150, 255), 20, 0);
				client->addCounter("b", QColor(150, 150, 150), 20, 0);
				client->addCounter("r", QColor(250, 150, 150), 20, 0);
				client->addCounter("g", QColor(150, 255, 150), 20, 0);
				client->addCounter("x", QColor(255, 255, 255), 20, 0);
				client->addCounter("storm", QColor(255, 255, 255), 20, 0);
				
				mulliganCards = 7;
			}

			break;
		}
		case eventDraw: {
			CardZone *deck = zones.value("deck");
			CardZone *hand = zones.value("hand");
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
			if (data.size() != 8)
				break;
			int cardId = data[0].toInt();
			QString cardName = data[1];
			CardZone *startZone = zones.value(data[2], 0);
			if (!startZone) {
				qDebug(QString("start zone invalid: %1").arg(data[2]).toLatin1());
				break;
			}
			int position = data[3].toInt();
			CardZone *targetZone = zones.value(data[4], 0);
			if (!targetZone) {
				qDebug(QString("target zone invalid: %1").arg(data[4]).toLatin1());
				break;
			}

			int x = data[5].toInt();
			int y = data[6].toInt();
			bool facedown = data[7].toInt();

			int logPosition = position;
			int logX = x;
			if (position == -1)
				position = 0;
			if (x == -1)
				x = 0;
			CardItem *card = startZone->takeCard(position, cardId, cardName, startZone != targetZone);
			if (!card) {
				qDebug("moveCard: card not found");
				break;
			}
			
			card->deleteDragItem();

			card->setFaceDown(facedown);

			// The log event has to be sent before the card is added to the target zone
			// because the addCard function can modify the card object.
			emit logMoveCard(this, card->getName(), startZone, logPosition, targetZone, logX);

			targetZone->addCard(card, true, x, y);

			// Look at all arrows from and to the card.
			// If the card was moved to another zone, delete the arrows, otherwise update them.
			QMapIterator<int, Player *> playerIterator(static_cast<Game *>(parent())->getPlayers());
			while (playerIterator.hasNext()) {
				Player *p = playerIterator.next().value();

				QList<int> arrowsToDelete;
				QMapIterator<int, ArrowItem *> arrowIterator(p->getArrows());
				while (arrowIterator.hasNext()) {
					ArrowItem *arrow = arrowIterator.next().value();
					if ((arrow->getStartItem() == card) || (arrow->getTargetItem() == card)) {
						if (startZone == targetZone)
							arrow->updatePath();
						else
							arrowsToDelete.append(arrow->getId());
					}
				}
				for (int i = 0; i < arrowsToDelete.size(); ++i)
					p->delArrow(arrowsToDelete[i]);
			}

			break;
		}
		case eventCreateToken: {
			// zone, cardid, cardname, powtough, x, y
			if (data.size() != 6)
				break;
			CardZone *zone = zones.value(data[0], 0);
			if (!zone)
				break;
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
			if (data.size() != 4)
				break;
			CardZone *zone = zones.value(data[0], 0);
			if (!zone)
				break;
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
			if (data.size() != 5)
				break;
			int counterId = data[0].toInt();
			QString counterName = data[1];
			int colorValue = data[2].toInt();
			int radius = data[3].toInt();
			int value = data[4].toInt();
			QColor color(colorValue / 65536, (colorValue % 65536) / 256, colorValue % 256);
			addCounter(counterId, counterName, color, radius, value);
			break;
		}
		case eventSetCounter: {
			if (data.size() != 2)
				break;
			int counterId = data[0].toInt();
			int value = data[1].toInt();
			Counter *c = counters.value(counterId, 0);
			if (!c)
				break;
			int oldValue = c->getValue();
			c->setValue(value);
			emit logSetCounter(this, c->getName(), value, oldValue);
			break;
		}
		case eventCreateArrow: {
			if (data.size() != 8)
				break;
			
			const QMap<int, Player *> &playerList = static_cast<Game *>(parent())->getPlayers();
			
			Player *startPlayer = playerList.value(data[1].toInt(), 0);
			Player *targetPlayer = playerList.value(data[4].toInt(), 0);
			if (!startPlayer || !targetPlayer)
				return;
			
			CardZone *startZone = startPlayer->getZones().value(data[2], 0);
			CardZone *targetZone = targetPlayer->getZones().value(data[5], 0);
			if (!startZone || !targetZone)
				return;
			
			CardItem *startCard = startZone->getCard(data[3].toInt(), QString());
			CardItem *targetCard = targetZone->getCard(data[6].toInt(), QString());
			if (!startCard || !targetCard)
				return;
	
			addArrow(data[0].toInt(), startCard, targetCard, client->numberToColor(data[7].toInt()));
			emit logCreateArrow(this, startPlayer, startCard->getName(), targetPlayer, targetCard->getName());
			
			break;
		}
		case eventDeleteArrow: {
			if (data.size() != 1)
				break;
			
			delArrow(data[0].toInt());
			break;
		}
		default:
			qDebug("unhandled player event");
	}
}
*/
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
	
	int totalWidth = CARD_WIDTH + counterAreaWidth + 5;
	
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

void Player::processPlayerInfo(ServerInfo_Player *info)
{
}

void Player::addCounter(int counterId, const QString &name, QColor color, int radius, int value)
{
	Counter *c = new Counter(this, counterId, name, color, radius, value, this);
	counters.insert(counterId, c);
	if (countersMenu)
		countersMenu->addMenu(c->getMenu());
	rearrangeCounters();
}

void Player::delCounter(int counterId)
{
	Counter *c = counters.value(counterId, 0);
	if (!c)
		return;
	counters.remove(counterId);
	delete c;
	rearrangeCounters();
}

void Player::clearCounters()
{
	QMapIterator<int, Counter *> counterIterator(counters);
	while (counterIterator.hasNext())
		delete counterIterator.next().value();
	counters.clear();
}

void Player::addArrow(int arrowId, CardItem *startCard, CardItem *targetCard, const QColor &color)
{
	ArrowItem *arrow = new ArrowItem(this, arrowId, startCard, targetCard, color);
	arrows.insert(arrowId, arrow);
	scene()->addItem(arrow);
}

void Player::delArrow(int arrowId)
{
	ArrowItem *a = arrows.value(arrowId, 0);
	if (!a)
		return;
	arrows.remove(arrowId);
	delete a;
}

void Player::clearArrows()
{
	QMapIterator<int, ArrowItem *> arrowIterator(arrows);
	while (arrowIterator.hasNext())
		delete arrowIterator.next().value();
	arrows.clear();
}

void Player::rearrangeCounters()
{
	qreal marginTop = 50;
	qreal marginBottom = 15;
	
	// Determine total height of bounding rectangles
	qreal totalHeight = 0;
	QMapIterator<int, Counter *> counterIterator(counters);
	while (counterIterator.hasNext()) {
		counterIterator.next();
		totalHeight += counterIterator.value()->boundingRect().height();
	}
	
	// Determine free space between objects
	qreal padding = (boundingRect().height() - marginTop - marginBottom - totalHeight) / (counters.size() - 1);
	qreal y = boundingRect().y() + marginTop;
	
	if (counters.size() == 1) {
		padding = 0;
		y += (boundingRect().height() - marginTop - marginBottom) / 2;
	}
	
	// Place objects
	for (counterIterator.toFront(); counterIterator.hasNext(); ) {
		Counter *c = counterIterator.next().value();

		QRectF br = c->boundingRect();
		c->setPos((counterAreaWidth - br.width()) / 2, y);
		y += br.height() + padding;
	}
}
