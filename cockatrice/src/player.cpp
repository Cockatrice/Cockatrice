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
#include "protocol_items.h"
#include "gamescene.h"
#include "settingscache.h"
#include <QSettings>
#include <QPainter>
#include <QMenu>

Player::Player(const QString &_name, int _id, bool _local, Client *_client, TabGame *_parent)
	: QObject(_parent), defaultNumberTopCards(3), name(_name), id(_id), active(false), local(_local), client(_client)
{
	setCacheMode(DeviceCoordinateCache);
	
	connect(settingsCache, SIGNAL(playerBgPathChanged()), this, SLOT(updateBgPixmap()));
	updateBgPixmap();
	
	QPointF base = QPointF(counterAreaWidth, 50);

	PileZone *deck = new PileZone(this, "deck", true, false, this);
	deck->setPos(base);

	qreal h = deck->boundingRect().height() + 20;

	PileZone *grave = new PileZone(this, "grave", false, true, this);
	grave->setPos(base + QPointF(0, h));

	PileZone *rfg = new PileZone(this, "rfg", false, true, this);
	rfg->setPos(base + QPointF(0, 2 * h));

	PileZone *sb = new PileZone(this, "sb", false, false, this);
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
		
		// Card menu
		aTap = new QAction(this);
		aUntap = new QAction(this);
		aDoesntUntap = new QAction(this);
		aFlip = new QAction(this);
		aAddCounter = new QAction(this);
		aRemoveCounter = new QAction(this);
		aSetCounters = new QAction(this);
		connect(aSetCounters, SIGNAL(triggered()), this, SLOT(actSetCounters()));
		aMoveToTopLibrary = new QAction(this);
		aMoveToBottomLibrary = new QAction(this);
		aMoveToGraveyard = new QAction(this);
		aMoveToExile = new QAction(this);
	
		cardMenu = new QMenu;
		cardMenu->addAction(aTap);
		cardMenu->addAction(aUntap);
		cardMenu->addAction(aDoesntUntap);
		cardMenu->addSeparator();
		cardMenu->addAction(aFlip);
		cardMenu->addSeparator();
		cardMenu->addAction(aAddCounter);
		cardMenu->addAction(aRemoveCounter);
		cardMenu->addAction(aSetCounters);
		cardMenu->addSeparator();
		moveMenu = cardMenu->addMenu(QString());

		playerMenu->addSeparator();
		playerMenu->addMenu(cardMenu);
		
		moveMenu->addAction(aMoveToTopLibrary);
		moveMenu->addAction(aMoveToBottomLibrary);
		moveMenu->addAction(aMoveToGraveyard);
		moveMenu->addAction(aMoveToExile);
		
		cardMenuHandlers.insert(aTap, &Player::actTap);
		cardMenuHandlers.insert(aUntap, &Player::actUntap);
		cardMenuHandlers.insert(aDoesntUntap, &Player::actDoesntUntap);
		cardMenuHandlers.insert(aFlip, &Player::actFlip);
		cardMenuHandlers.insert(aAddCounter, &Player::actAddCounter);
		cardMenuHandlers.insert(aRemoveCounter, &Player::actRemoveCounter);
		cardMenuHandlers.insert(aMoveToTopLibrary, &Player::actMoveToTopLibrary);
		cardMenuHandlers.insert(aMoveToBottomLibrary, &Player::actMoveToBottomLibrary);
		cardMenuHandlers.insert(aMoveToGraveyard, &Player::actMoveToGraveyard);
		cardMenuHandlers.insert(aMoveToExile, &Player::actMoveToExile);
		
		QHashIterator<QAction *, CardMenuHandler> i(cardMenuHandlers);
		while (i.hasNext()) {
			i.next();
			connect(i.key(), SIGNAL(triggered()), this, SLOT(cardMenuAction()));
		}
	} else {
		countersMenu = 0;
		sbMenu = 0;
		cardMenu = 0;
	}
	
	retranslateUi();
}

Player::~Player()
{
	qDebug("Player destructor");

	static_cast<GameScene *>(scene())->removePlayer(this);
	
	QMapIterator<QString, CardZone *> i(zones);
	while (i.hasNext())
		delete i.next().value();

	clearCounters();
	clearArrows();
	delete playerMenu;
	delete cardMenu;
}

void Player::updateBgPixmap()
{
	QString bgPath = settingsCache->getPlayerBgPath();
	if (!bgPath.isEmpty()) {
		qDebug() << "loading" << bgPath;
		bgPixmap.load(bgPath);
	}
	update();
}

void Player::updateBoundingRect()
{
	prepareGeometryChange();
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
		aViewTopCards->setShortcut(tr("Ctrl+W"));
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

		cardMenu->setTitle(tr("C&ard"));
		aTap->setText(tr("&Tap"));
		aUntap->setText(tr("&Untap"));
		aDoesntUntap->setText(tr("Toggle &normal untapping"));
		aFlip->setText(tr("&Flip"));
		aAddCounter->setText(tr("&Add counter"));
		aRemoveCounter->setText(tr("&Remove counter"));
		aSetCounters->setText(tr("&Set counters..."));
		aMoveToTopLibrary->setText(tr("&top of library"));
		aMoveToBottomLibrary->setText(tr("&bottom of library"));
		aMoveToGraveyard->setText(tr("&graveyard"));
		aMoveToGraveyard->setShortcut(tr("Ctrl+Del"));
		aMoveToExile->setText(tr("&exile"));
		
		moveMenu->setTitle(tr("&Move to"));
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
	static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", -1);
}

void Player::actViewTopCards()
{
	bool ok;
	int number = QInputDialog::getInteger(0, tr("View top cards of library"), tr("Number of cards:"), defaultNumberTopCards, 1, 2000000000, 1, &ok);
	if (ok) {
		defaultNumberTopCards = number;
		static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", number);
	}
}

void Player::actViewGraveyard()
{
	static_cast<GameScene *>(scene())->toggleZoneView(this, "grave", -1);
}

void Player::actViewRfg()
{
	static_cast<GameScene *>(scene())->toggleZoneView(this, "rfg", -1);
}

void Player::actViewSideboard()
{
	static_cast<GameScene *>(scene())->toggleZoneView(this, "sb", -1);
}

void Player::actShuffle()
{
	sendGameCommand(new Command_Shuffle);
}

void Player::actDrawCard()
{
	sendGameCommand(new Command_DrawCards(-1, 1));
}

void Player::actMulligan()
{
	sendGameCommand(new Command_Mulligan);
}

void Player::actDrawCards()
{
	int number = QInputDialog::getInteger(0, tr("Draw cards"), tr("Number:"));
        if (number)
		sendGameCommand(new Command_DrawCards(-1, number));
}

void Player::actUntapAll()
{
	sendGameCommand(new Command_SetCardAttr(-1, "table", -1, "tapped", "0"));
}

void Player::actRollDie()
{
	bool ok;
	int sides = QInputDialog::getInteger(0, tr("Roll die"), tr("Number of sides:"), 20, 2, 1000, 1, &ok);
	if (ok)
		sendGameCommand(new Command_RollDie(-1, sides));
}

void Player::actCreateToken()
{
	QString cardname = QInputDialog::getText(0, tr("Create token"), tr("Name:"));
	if (!cardname.isEmpty())
		sendGameCommand(new Command_CreateToken(-1, "table", cardname, QString(), 0, 0));
}

void Player::actSayMessage()
{
	QAction *a = qobject_cast<QAction *>(sender());
	sendGameCommand(new Command_Say(-1, a->text()));
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

void Player::eventSay(Event_Say *event)
{
	emit logSay(this, event->getMessage());
}

void Player::eventShuffle(Event_Shuffle * /*event*/)
{
	emit logShuffle(this);
}

void Player::eventRollDie(Event_RollDie *event)
{
	emit logRollDie(this, event->getSides(), event->getValue());
}

void Player::eventCreateArrows(Event_CreateArrows *event)
{
	const QList<ServerInfo_Arrow *> eventArrowList = event->getArrowList();
	for (int i = 0; i < eventArrowList.size(); ++i) {
		ArrowItem *arrow = addArrow(eventArrowList[i]);
		if (!arrow)
			return;
		emit logCreateArrow(this, arrow->getStartItem()->getOwner(), arrow->getStartItem()->getName(), arrow->getTargetItem()->getOwner(), arrow->getTargetItem()->getName());
	}
}

void Player::eventDeleteArrow(Event_DeleteArrow *event)
{
	delArrow(event->getArrowId());
}

void Player::eventCreateToken(Event_CreateToken *event)
{
	CardZone *zone = zones.value(event->getZone(), 0);
	if (!zone)
		return;

	CardItem *card = new CardItem(this, event->getCardName(), event->getCardId());

	emit logCreateToken(this, card->getName());
	zone->addCard(card, true, event->getX(), event->getY());

}

void Player::eventSetCardAttr(Event_SetCardAttr *event)
{
	CardZone *zone = zones.value(event->getZone(), 0);
	if (!zone)
		return;

	if (event->getCardId() == -1) {
		const CardList &cards = zone->getCards();
		for (int i = 0; i < cards.size(); i++)
			setCardAttrHelper(cards.at(i), event->getAttrName(), event->getAttrValue(), true);
		if (event->getAttrName() == "tapped")
			emit logSetTapped(this, QString("-1"), event->getAttrValue() == "1");
	} else {
		CardItem *card = zone->getCard(event->getCardId(), QString());
		if (!card) {
			qDebug() << "Player::eventSetCardAttr: card id=" << event->getCardId() << "not found";
			return;
		}
		setCardAttrHelper(card, event->getAttrName(), event->getAttrValue(), false);
	}
}

void Player::eventCreateCounters(Event_CreateCounters *event)
{
	const QList<ServerInfo_Counter *> &eventCounterList = event->getCounterList();
	for (int i = 0; i < eventCounterList.size(); ++i)
		addCounter(eventCounterList[i]);
}

void Player::eventSetCounter(Event_SetCounter *event)
{
	Counter *c = counters.value(event->getCounterId(), 0);
	if (!c)
		return;
	int oldValue = c->getValue();
	c->setValue(event->getValue());
	emit logSetCounter(this, c->getName(), event->getValue(), oldValue);
}

void Player::eventDelCounter(Event_DelCounter *event)
{
	delCounter(event->getCounterId());
}

void Player::eventDumpZone(Event_DumpZone *event)
{
	Player *zoneOwner = static_cast<TabGame *>(parent())->getPlayers().value(event->getZoneOwnerId(), 0);
	if (!zoneOwner)
		return;
	CardZone *zone = zoneOwner->getZones().value(event->getZone(), 0);
	if (!zone)
		return;
	emit logDumpZone(this, zone, event->getNumberCards());
}

void Player::eventStopDumpZone(Event_StopDumpZone *event)
{
	Player *zoneOwner = static_cast<TabGame *>(parent())->getPlayers().value(event->getZoneOwnerId(), 0);
	if (!zoneOwner)
		return;
	CardZone *zone = zoneOwner->getZones().value(event->getZone(), 0);
	if (!zone)
		return;
	emit logStopDumpZone(this, zone);
}

void Player::eventMoveCard(Event_MoveCard *event)
{
	CardZone *startZone = zones.value(event->getStartZone(), 0);
	CardZone *targetZone = zones.value(event->getTargetZone(), 0);
	if (!startZone || !targetZone)
		return;
	
	int position = event->getPosition();
	int x = event->getX();
	int y = event->getY();

	int logPosition = position;
	int logX = x;
	if (x == -1)
		x = 0;
	CardItem *card = startZone->takeCard(position, event->getCardId(), event->getCardName(), startZone != targetZone);
	if (!card)
		return;
	
	card->deleteDragItem();

	card->setId(event->getNewCardId());
	card->setFaceDown(event->getFaceDown());
	if (startZone != targetZone)
		card->setBeingPointedAt(false);

	// The log event has to be sent before the card is added to the target zone
	// because the addCard function can modify the card object.
	emit logMoveCard(this, card->getName(), startZone, logPosition, targetZone, logX);

	targetZone->addCard(card, true, x, y);

	// Look at all arrows from and to the card.
	// If the card was moved to another zone, delete the arrows, otherwise update them.
	QMapIterator<int, Player *> playerIterator(static_cast<TabGame *>(parent())->getPlayers());
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
}

void Player::eventDrawCards(Event_DrawCards *event)
{
	CardZone *deck = zones.value("deck");
	CardZone *hand = zones.value("hand");
	const QList<ServerInfo_Card *> &cardList = event->getCardList();
	if (!cardList.isEmpty())
		for (int i = 0; i < cardList.size(); ++i)
			hand->addCard(deck->takeCard(0, cardList[i]->getId(), cardList[i]->getName()), false, -1);
	else
		for (int i = 0; i < event->getNumberCards(); ++i)
			hand->addCard(deck->takeCard(0, -1, QString()), false, -1);
	hand->reorganizeCards();
	deck->reorganizeCards();
	
	emit logDrawCards(this, event->getNumberCards());
}

void Player::processGameEvent(GameEvent *event, GameEventContext *context)
{
	switch (event->getItemId()) {
		case ItemId_Event_Say: eventSay(qobject_cast<Event_Say *>(event)); break;
		case ItemId_Event_Shuffle: eventShuffle(qobject_cast<Event_Shuffle *>(event)); break;
		case ItemId_Event_RollDie: eventRollDie(qobject_cast<Event_RollDie *>(event)); break;
		case ItemId_Event_CreateArrows: eventCreateArrows(qobject_cast<Event_CreateArrows *>(event)); break;
		case ItemId_Event_DeleteArrow: eventDeleteArrow(qobject_cast<Event_DeleteArrow *>(event)); break;
		case ItemId_Event_CreateToken: eventCreateToken(qobject_cast<Event_CreateToken *>(event)); break;
		case ItemId_Event_SetCardAttr: eventSetCardAttr(qobject_cast<Event_SetCardAttr *>(event)); break;
		case ItemId_Event_CreateCounters: eventCreateCounters(qobject_cast<Event_CreateCounters *>(event)); break;
		case ItemId_Event_SetCounter: eventSetCounter(qobject_cast<Event_SetCounter *>(event)); break;
		case ItemId_Event_DelCounter: eventDelCounter(qobject_cast<Event_DelCounter *>(event)); break;
		case ItemId_Event_DumpZone: eventDumpZone(qobject_cast<Event_DumpZone *>(event)); break;
		case ItemId_Event_StopDumpZone: eventStopDumpZone(qobject_cast<Event_StopDumpZone *>(event)); break;
		case ItemId_Event_MoveCard: eventMoveCard(qobject_cast<Event_MoveCard *>(event)); break;
		case ItemId_Event_DrawCards: eventDrawCards(qobject_cast<Event_DrawCards *>(event)); break;
		default: {
			qDebug() << "unhandled game event";
		}
	}
}

void Player::showCardMenu(const QPoint &p)
{
	if (cardMenu)
		cardMenu->exec(p);
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
	QMapIterator<QString, CardZone *> zoneIt(zones);
	while (zoneIt.hasNext())
		zoneIt.next().value()->clearContents();

	for (int i = 0; i < info->getZoneList().size(); ++i) {
		ServerInfo_Zone *zoneInfo = info->getZoneList()[i];
		CardZone *zone = zones.value(zoneInfo->getName(), 0);
		if (!zone)
			continue;

		const QList<ServerInfo_Card *> &cardList = zoneInfo->getCardList();
		if (cardList.isEmpty()) {
			for (int j = 0; j < zoneInfo->getCardCount(); ++j)
				zone->addCard(new CardItem(this), false, -1);
		} else {
			for (int j = 0; j < cardList.size(); ++j) {
				CardItem *card = new CardItem(this);
				card->processCardInfo(cardList[j]);
				zone->addCard(card, false, cardList[j]->getX(), cardList[j]->getY());
			}
		}
		zone->reorganizeCards();
	}

	clearCounters();
	for (int i = 0; i < info->getCounterList().size(); ++i) {
		addCounter(info->getCounterList().at(i));
	}

	clearArrows();
	for (int i = 0; i < info->getArrowList().size(); ++i)
		addArrow(info->getArrowList().at(i));
}

void Player::addCard(CardItem *c)
{
	emit newCardAdded(c);
}

void Player::addZone(CardZone *z)
{
	zones.insert(z->getName(), z);
}

Counter *Player::addCounter(ServerInfo_Counter *counter)
{
	return addCounter(counter->getId(), counter->getName(), counter->getColor(), counter->getRadius(), counter->getCount());
}

Counter *Player::addCounter(int counterId, const QString &name, QColor color, int radius, int value)
{
	Counter *c = new Counter(this, counterId, name, color, radius, value, this);
	counters.insert(counterId, c);
	if (countersMenu)
		countersMenu->addMenu(c->getMenu());
	rearrangeCounters();
	return c;
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

ArrowItem *Player::addArrow(ServerInfo_Arrow *arrow)
{
	const QMap<int, Player *> &playerList = static_cast<TabGame *>(parent())->getPlayers();
	Player *startPlayer = playerList.value(arrow->getStartPlayerId(), 0);
	Player *targetPlayer = playerList.value(arrow->getTargetPlayerId(), 0);
	if (!startPlayer || !targetPlayer)
		return 0;
	
	CardZone *startZone = startPlayer->getZones().value(arrow->getStartZone(), 0);
	CardZone *targetZone = targetPlayer->getZones().value(arrow->getTargetZone(), 0);
	if (!startZone || !targetZone)
		return 0;
	
	CardItem *startCard = startZone->getCard(arrow->getStartCardId(), QString());
	CardItem *targetCard = targetZone->getCard(arrow->getTargetCardId(), QString());
	if (!startCard || !targetCard)
		return 0;
	
	return addArrow(arrow->getId(), startCard, targetCard, arrow->getColor());
}

ArrowItem *Player::addArrow(int arrowId, CardItem *startCard, CardItem *targetCard, const QColor &color)
{
	ArrowItem *arrow = new ArrowItem(this, arrowId, startCard, targetCard, color);
	arrows.insert(arrowId, arrow);
	scene()->addItem(arrow);
	return arrow;
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

void Player::sendGameCommand(GameCommand *command)
{
	static_cast<TabGame *>(parent())->sendGameCommand(command);
}

void Player::sendCommandContainer(CommandContainer *cont)
{
	static_cast<TabGame *>(parent())->sendCommandContainer(cont);
}

void Player::cardMenuAction()
{
	// Determine the appropriate handler function.
	CardMenuHandler handler = cardMenuHandlers.value(static_cast<QAction *>(sender()));
			
	// The list of selected items is randomly shuffled.
	QList<QGraphicsItem *> sel = scene()->selectedItems();
	while (!sel.isEmpty()) {
		unsigned int i = (unsigned int) (((double) sel.size()) * qrand() / (RAND_MAX + 1.0));
		CardItem *card = qgraphicsitem_cast<CardItem *>(sel.takeAt(i));
		// For each item, the handler function is called.
		(this->*handler)(card);
	}
}

void Player::actTap(CardItem *card)
{
	if (!card->getTapped())
		sendGameCommand(new Command_SetCardAttr(-1, qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "tapped", "1"));
}

void Player::actUntap(CardItem *card)
{
	if (card->getTapped())
		sendGameCommand(new Command_SetCardAttr(-1, qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "tapped", "0"));
}

void Player::actDoesntUntap(CardItem *card)
{
	sendGameCommand(new Command_SetCardAttr(-1, qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "doesnt_untap", QString::number(!card->getDoesntUntap())));
}

void Player::actFlip(CardItem *card)
{
	QString zone = qgraphicsitem_cast<CardZone *>(card->parentItem())->getName();
	sendGameCommand(new Command_MoveCard(-1, zone, card->getId(), zone, card->getGridPoint().x(), card->getGridPoint().y(), !card->getFaceDown()));
}

void Player::actAddCounter(CardItem *card)
{
	if (card->getCounters() < MAX_COUNTERS_ON_CARD)
		sendGameCommand(new Command_SetCardAttr(-1, qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "counters", QString::number(card->getCounters() + 1)));
}

void Player::actRemoveCounter(CardItem *card)
{
	if (card->getCounters())
		sendGameCommand(new Command_SetCardAttr(-1, qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "counters", QString::number(card->getCounters() - 1)));
}

void Player::actSetCounters()
{
	bool ok;
	int number = QInputDialog::getInteger(0, tr("Set counters"), tr("Number:"), 0, 0, MAX_COUNTERS_ON_CARD, 1, &ok);
	if (!ok)
		return;

	QListIterator<QGraphicsItem *> i(scene()->selectedItems());
	while (i.hasNext()) {
		CardItem *temp = (CardItem *) i.next();
		sendGameCommand(new Command_SetCardAttr(-1, qgraphicsitem_cast<CardZone *>(temp->parentItem())->getName(), temp->getId(), "counters", QString::number(number)));
	}
}

void Player::actMoveToTopLibrary(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	sendGameCommand(new Command_MoveCard(-1, startZone->getName(), card->getId(), "deck", 0, 0, false));
}

void Player::actMoveToBottomLibrary(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	sendGameCommand(new Command_MoveCard(-1, startZone->getName(), card->getId(), "deck", -1, 0, false));
}

void Player::actMoveToGraveyard(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	sendGameCommand(new Command_MoveCard(-1, startZone->getName(), card->getId(), "grave", 0, 0, false));
}

void Player::actMoveToExile(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	sendGameCommand(new Command_MoveCard(-1, startZone->getName(), card->getId(), "rfg", 0, 0, false));
}
