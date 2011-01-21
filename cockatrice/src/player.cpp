#include "player.h"
#include "cardzone.h"
#include "playertarget.h"
#include "counter_general.h"
#include "arrowitem.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "pilezone.h"
#include "stackzone.h"
#include "tablezone.h"
#include "handzone.h"
#include "handcounter.h"
#include "cardlist.h"
#include "tab_game.h"
#include "protocol_items.h"
#include "gamescene.h"
#include "settingscache.h"
#include "dlg_create_token.h"
#include "carddatabase.h"
#include <QSettings>
#include <QPainter>
#include <QMenu>
#include <QDebug>

Player::Player(ServerInfo_User *info, int _id, bool _local, TabGame *_parent)
	: QObject(_parent), shortcutsActive(false), defaultNumberTopCards(3), lastTokenDestroy(true), userInfo(new ServerInfo_User(info)), id(_id), active(false), local(_local), mirrored(false), dialogSemaphore(false)
{
	setCacheMode(DeviceCoordinateCache);
	
	connect(settingsCache, SIGNAL(horizontalHandChanged()), this, SLOT(rearrangeZones()));
	connect(settingsCache, SIGNAL(playerBgPathChanged()), this, SLOT(updateBgPixmap()));
	updateBgPixmap();
	
	playerTarget = new PlayerTarget(this);
	qreal avatarMargin = (counterAreaWidth + CARD_HEIGHT + 15 - playerTarget->boundingRect().width()) / 2.0;
	playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

	PileZone *deck = new PileZone(this, "deck", true, false, this);
	QPointF base = QPointF(counterAreaWidth + (CARD_HEIGHT - CARD_WIDTH + 15) / 2.0, 10 + playerTarget->boundingRect().height() + 5 - (CARD_HEIGHT - CARD_WIDTH) / 2.0);
	deck->setPos(base);

	qreal h = deck->boundingRect().width() + 5;

	HandCounter *handCounter = new HandCounter(this);
	handCounter->setPos(base + QPointF(0, h + 10));
	qreal h2 = handCounter->boundingRect().height();
	
	PileZone *grave = new PileZone(this, "grave", false, true, this);
	grave->setPos(base + QPointF(0, h + h2 + 10));

	PileZone *rfg = new PileZone(this, "rfg", false, true, this);
	rfg->setPos(base + QPointF(0, 2 * h + h2 + 10));

	PileZone *sb = new PileZone(this, "sb", false, false, this);
	sb->setVisible(false);

	table = new TableZone(this, this);
	connect(table, SIGNAL(sizeChanged()), this, SLOT(updateBoundingRect()));
	
	stack = new StackZone(this, (int) table->boundingRect().height(), this);
	
	hand = new HandZone(this, _local || (_parent->getSpectator() && _parent->getSpectatorsSeeEverything()), (int) table->boundingRect().height(), this);
	connect(hand, SIGNAL(cardCountChanged()), handCounter, SLOT(updateNumber()));
	
	updateBoundingRect();

	if (local) {
		connect(_parent, SIGNAL(playerAdded(Player *)), this, SLOT(addPlayer(Player *)));
		connect(_parent, SIGNAL(playerRemoved(Player *)), this, SLOT(removePlayer(Player *)));
		
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
		aMoveTopCardsToGrave = new QAction(this);
		connect(aMoveTopCardsToGrave, SIGNAL(triggered()), this, SLOT(actMoveTopCardsToGrave()));
		aMoveTopCardsToExile = new QAction(this);
		connect(aMoveTopCardsToExile, SIGNAL(triggered()), this, SLOT(actMoveTopCardsToExile()));
		aMoveTopCardToBottom = new QAction(this);
		connect(aMoveTopCardToBottom, SIGNAL(triggered()), this, SLOT(actMoveTopCardToBottom()));
	}

	playerMenu = new QMenu(QString());

	if (local) {
		handMenu = playerMenu->addMenu(QString());
                handMenu->addAction(aMulligan);
		handMenu->addAction(aMoveHandToTopLibrary);
		handMenu->addAction(aMoveHandToBottomLibrary);
		handMenu->addAction(aMoveHandToGrave);
		handMenu->addAction(aMoveHandToRfg);
		handMenu->addSeparator();
		playerLists.append(mRevealHand = handMenu->addMenu(QString()));
		playerLists.append(mRevealRandomHandCard = handMenu->addMenu(QString()));
		hand->setMenu(handMenu);

		libraryMenu = playerMenu->addMenu(QString());
		libraryMenu->addAction(aDrawCard);
		libraryMenu->addAction(aDrawCards);
		libraryMenu->addSeparator();
		libraryMenu->addAction(aShuffle);
		libraryMenu->addSeparator();
		libraryMenu->addAction(aViewLibrary);
		libraryMenu->addAction(aViewTopCards);
		libraryMenu->addSeparator();
		playerLists.append(mRevealLibrary = libraryMenu->addMenu(QString()));
		playerLists.append(mRevealTopCard = libraryMenu->addMenu(QString()));
		libraryMenu->addSeparator();
		libraryMenu->addAction(aMoveTopCardsToGrave);
		libraryMenu->addAction(aMoveTopCardsToExile);
		libraryMenu->addAction(aMoveTopCardToBottom);
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
		
		aCreateAnotherToken = new QAction(this);
		connect(aCreateAnotherToken, SIGNAL(triggered()), this, SLOT(actCreateAnotherToken()));
		aCreateAnotherToken->setEnabled(false);

		playerMenu->addSeparator();
		countersMenu = playerMenu->addMenu(QString());
		playerMenu->addSeparator();
		playerMenu->addAction(aUntapAll);
		playerMenu->addSeparator();
		playerMenu->addAction(aRollDie);
		playerMenu->addSeparator();
		playerMenu->addAction(aCreateToken);
		playerMenu->addAction(aCreateAnotherToken);
		playerMenu->addSeparator();
		sayMenu = playerMenu->addMenu(QString());
		initSayMenu();
		
		aCardMenu = new QAction(this);
		playerMenu->addSeparator();
		playerMenu->addAction(aCardMenu);

		for (int i = 0; i < playerLists.size(); ++i) {
			QAction *newAction = playerLists[i]->addAction(QString());
			newAction->setData(-1);
			connect(newAction, SIGNAL(triggered()), this, SLOT(playerListActionTriggered()));
			allPlayersActions.append(newAction);
			playerLists[i]->addSeparator();
		}
	
	} else {
		countersMenu = 0;
		sbMenu = 0;
		aCreateAnotherToken = 0;
		aCardMenu = 0;
	}
	
	const QList<Player *> &players = _parent->getPlayers().values();
	for (int i = 0; i < players.size(); ++i)
		addPlayer(players[i]);
	
	rearrangeZones();
	retranslateUi();
}

Player::~Player()
{
	qDebug() << "Player destructor:" << getName();

	static_cast<GameScene *>(scene())->removePlayer(this);
	
	clearArrows();

	QMapIterator<QString, CardZone *> i(zones);
	while (i.hasNext())
		delete i.next().value();

	clearCounters();
	delete playerMenu;
	delete userInfo;
}

void Player::addPlayer(Player *player)
{
	if (player == this)
		return;
	for (int i = 0; i < playerLists.size(); ++i) {
		QAction *newAction = playerLists[i]->addAction(player->getName());
		newAction->setData(player->getId());
		connect(newAction, SIGNAL(triggered()), this, SLOT(playerListActionTriggered()));
	}
}

void Player::removePlayer(Player *player)
{
	for (int i = 0; i < playerLists.size(); ++i) {
		QList<QAction *> actionList = playerLists[i]->actions();
		for (int j = 0; j < actionList.size(); ++j)
			if (actionList[j]->data().toInt() == player->getId()) {
				playerLists[i]->removeAction(actionList[j]);
				actionList[j]->deleteLater();
			}
	}
}

void Player::playerListActionTriggered()
{
	QAction *action = static_cast<QAction *>(sender());
	QMenu *menu = static_cast<QMenu *>(action->parentWidget());
	int otherPlayerId = action->data().toInt();
	
	if (menu == mRevealLibrary) {
		sendGameCommand(new Command_RevealCards(-1, "deck", -1, otherPlayerId));
	} else if (menu == mRevealTopCard) {
		sendGameCommand(new Command_RevealCards(-1, "deck", 0, otherPlayerId));
	} else if (menu == mRevealHand) {
		sendGameCommand(new Command_RevealCards(-1, "hand", -1, otherPlayerId));
	} else if (menu == mRevealRandomHandCard) {
		sendGameCommand(new Command_RevealCards(-1, "hand", -2, otherPlayerId));
	}
}

void Player::rearrangeZones()
{
	QPointF base = QPointF(CARD_HEIGHT + counterAreaWidth + 15, 0);
	
	if (settingsCache->getHorizontalHand()) {
		if (mirrored) {
			if (hand->contentsKnown()) {
				hand->setVisible(true);
				hand->setPos(base);
				base += QPointF(0, hand->boundingRect().height());
			} else
				hand->setVisible(false);

			stack->setPos(base);
			base += QPointF(stack->boundingRect().width(), 0);
	
			table->setPos(base);
		} else {
			stack->setPos(base);
	
			table->setPos(base.x() + stack->boundingRect().width(), 0);
			base += QPointF(0, table->boundingRect().height());
			
			if (hand->contentsKnown()) {
				hand->setVisible(true);
				hand->setPos(base);
			} else
				hand->setVisible(false);
		}
		hand->setWidth(table->getWidth() + stack->boundingRect().width());
	} else {
		hand->setVisible(true);
		
		hand->setPos(base);
		base += QPointF(hand->boundingRect().width(), 0);
		
		stack->setPos(base);
		base += QPointF(stack->boundingRect().width(), 0);
	
		table->setPos(base);
	}
	hand->updateOrientation();
	table->reorganizeCards();
	updateBoundingRect();
	rearrangeCounters();
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
	qreal width = CARD_HEIGHT + 15 + counterAreaWidth + stack->boundingRect().width();
	if (settingsCache->getHorizontalHand()) {
		qreal handHeight = hand->isVisible() ? hand->boundingRect().height() : 0;
		bRect = QRectF(0, 0, width + table->boundingRect().width(), table->boundingRect().height() + handHeight);
	} else
		bRect = QRectF(0, 0, width + hand->boundingRect().width() + table->boundingRect().width(), table->boundingRect().height());
	emit sizeChanged();
}

void Player::retranslateUi()
{
	aViewGraveyard->setText(tr("&View graveyard"));
	aViewRfg->setText(tr("&View exile"));
	playerMenu->setTitle(tr("Player \"%1\"").arg(userInfo->getName()));
	graveMenu->setTitle(tr("&Graveyard"));
	rfgMenu->setTitle(tr("&Exile"));
	
	if (local) {
		aMoveHandToTopLibrary->setText(tr("Move to &top of library"));
		aMoveHandToBottomLibrary->setText(tr("Move to &bottom of library"));
		aMoveHandToGrave->setText(tr("Move to &graveyard"));
		aMoveHandToRfg->setText(tr("Move to &exile"));
		aMoveGraveToTopLibrary->setText(tr("Move to &top of library"));
		aMoveGraveToBottomLibrary->setText(tr("Move to &bottom of library"));
		aMoveGraveToHand->setText(tr("Move to &hand"));
		aMoveGraveToRfg->setText(tr("Move to &exile"));
		aMoveRfgToTopLibrary->setText(tr("Move to &top of library"));
		aMoveRfgToBottomLibrary->setText(tr("Move to &bottom of library"));
		aMoveRfgToHand->setText(tr("Move to &hand"));
		aMoveRfgToGrave->setText(tr("Move to &graveyard"));
		aViewLibrary->setText(tr("&View library"));
		aViewTopCards->setText(tr("View &top cards of library..."));
		mRevealLibrary->setTitle(tr("Reveal &library to"));
		mRevealTopCard->setTitle(tr("Reveal t&op card to"));
		aViewSideboard->setText(tr("&View sideboard"));
		aDrawCard->setText(tr("&Draw card"));
		aDrawCards->setText(tr("D&raw cards..."));
		aMulligan->setText(tr("Take &mulligan"));
		aShuffle->setText(tr("&Shuffle"));
		aMoveTopCardsToGrave->setText(tr("Move top cards to &graveyard..."));
		aMoveTopCardsToExile->setText(tr("Move top cards to &exile..."));
		aMoveTopCardToBottom->setText(tr("Put top card on &bottom"));
	
		handMenu->setTitle(tr("&Hand"));
		mRevealHand->setTitle(tr("&Reveal to"));
		mRevealRandomHandCard->setTitle(tr("Reveal r&andom card to"));
		sbMenu->setTitle(tr("&Sideboard"));
		libraryMenu->setTitle(tr("&Library"));
		countersMenu->setTitle(tr("&Counters"));

		aUntapAll->setText(tr("&Untap all permanents"));
		aRollDie->setText(tr("R&oll die..."));
		aCreateToken->setText(tr("&Create token..."));
		aCreateAnotherToken->setText(tr("C&reate another token"));
		sayMenu->setTitle(tr("S&ay"));
		
		QMapIterator<int, AbstractCounter *> counterIterator(counters);
		while (counterIterator.hasNext())
			counterIterator.next().value()->retranslateUi();

		aCardMenu->setText(tr("C&ard"));
		
		for (int i = 0; i < allPlayersActions.size(); ++i)
			allPlayersActions[i]->setText(tr("&All players"));
	}
	
	QMapIterator<QString, CardZone *> zoneIterator(zones);
	while (zoneIterator.hasNext())
		zoneIterator.next().value()->retranslateUi();
}

void Player::setShortcutsActive()
{
	shortcutsActive = true;
	
	aViewLibrary->setShortcut(tr("F3"));
	aViewTopCards->setShortcut(tr("Ctrl+W"));
	aViewGraveyard->setShortcut(tr("F4"));
	aDrawCard->setShortcut(tr("Ctrl+D"));
	aDrawCards->setShortcut(tr("Ctrl+E"));
	aMulligan->setShortcut(tr("Ctrl+M"));
	aShuffle->setShortcut(tr("Ctrl+S"));
	aUntapAll->setShortcut(tr("Ctrl+U"));
	aRollDie->setShortcut(tr("Ctrl+I"));
	aCreateToken->setShortcut(tr("Ctrl+T"));
	aCreateAnotherToken->setShortcut(tr("Ctrl+G"));

	QMapIterator<int, AbstractCounter *> counterIterator(counters);
	while (counterIterator.hasNext())
		counterIterator.next().value()->setShortcutsActive();
}

void Player::setShortcutsInactive()
{
	shortcutsActive = false;
	
	aViewLibrary->setShortcut(QKeySequence());
	aViewTopCards->setShortcut(QKeySequence());
	aViewGraveyard->setShortcut(QKeySequence());
	aDrawCard->setShortcut(QKeySequence());
	aDrawCards->setShortcut(QKeySequence());
	aMulligan->setShortcut(QKeySequence());
	aShuffle->setShortcut(QKeySequence());
	aUntapAll->setShortcut(QKeySequence());
	aRollDie->setShortcut(QKeySequence());
	aCreateToken->setShortcut(QKeySequence());
	aCreateAnotherToken->setShortcut(QKeySequence());

	QMapIterator<int, AbstractCounter *> counterIterator(counters);
	while (counterIterator.hasNext())
		counterIterator.next().value()->setShortcutsInactive();
}

void Player::initSayMenu()
{
	sayMenu->clear();

	QSettings settings;
	settings.beginGroup("messages");
	int count = settings.value("count", 0).toInt();
	for (int i = 0; i < count; i++) {
		QAction *newAction = new QAction(settings.value(QString("msg%1").arg(i)).toString(), this);
		if (i <= 10)
			newAction->setShortcut(QString("Ctrl+%1").arg((i + 1) % 10));
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

void Player::actMoveTopCardsToGrave()
{
	int number = QInputDialog::getInteger(0, tr("Move top cards to grave"), tr("Number:"));
        if (!number)
		return;

	QList<Command *> commandList;
	const int maxCards = zones.value("deck")->getCards().size();
	if (number > maxCards)
		number = maxCards;
	QList<CardId *> idList;
	for (int i = 0; i < number; ++i)
		idList.append(new CardId(i));
	sendGameCommand(new Command_MoveCard(-1, "deck", idList, getId(), "grave", 0, 0, false));
}

void Player::actMoveTopCardsToExile()
{
	int number = QInputDialog::getInteger(0, tr("Move top cards to exile"), tr("Number:"));
        if (!number)
		return;

	QList<Command *> commandList;
	const int maxCards = zones.value("deck")->getCards().size();
	if (number > maxCards)
		number = maxCards;
	QList<CardId *> idList;
	for (int i = 0; i < number; ++i)
		idList.append(new CardId(i));
	sendGameCommand(new Command_MoveCard(-1, "deck", idList, getId(), "rfg", 0, 0, false));
}

void Player::actMoveTopCardToBottom()
{
	sendGameCommand(new Command_MoveCard(-1, "deck", QList<CardId *>() << new CardId(0), getId(), "deck", -1, 0, false));
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
	DlgCreateToken dlg;
	if (!dlg.exec())
		return;
	
	lastTokenName = dlg.getName();
	lastTokenColor = dlg.getColor();
	lastTokenPT = dlg.getPT();
	lastTokenAnnotation = dlg.getAnnotation();
	lastTokenDestroy = dlg.getDestroy();
	aCreateAnotherToken->setEnabled(true);
	
	sendGameCommand(new Command_CreateToken(-1, "table", dlg.getName(), dlg.getColor(), dlg.getPT(), dlg.getAnnotation(), dlg.getDestroy(), -1, 0));
}

void Player::actCreateAnotherToken()
{
	sendGameCommand(new Command_CreateToken(-1, "table", lastTokenName, lastTokenColor, lastTokenPT, lastTokenAnnotation, lastTokenDestroy, -1, 0));
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
			card->setTapped(tapped, true);
		}
	} else if (aname == "attacking")
		card->setAttacking(avalue == "1");
	else if (aname == "facedown")
		card->setFaceDown(avalue == "1");
	else if (aname == "annotation") {
		emit logSetAnnotation(this, card->getName(), avalue);
		card->setAnnotation(avalue);
	} else if (aname == "doesnt_untap") {
		bool value = (avalue == "1");
		emit logSetDoesntUntap(this, card->getName(), value);
		card->setDoesntUntap(value);
	} else if (aname == "pt") {
		emit logSetPT(this, card->getName(), avalue);
		card->setPT(avalue);
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
		
		CardItem *startCard = static_cast<CardItem *>(arrow->getStartItem());
		CardItem *targetCard = qgraphicsitem_cast<CardItem *>(arrow->getTargetItem());
		if (targetCard)
			emit logCreateArrow(this, startCard->getOwner(), startCard->getName(), targetCard->getOwner(), targetCard->getName(), false);
		else
			emit logCreateArrow(this, startCard->getOwner(), startCard->getName(), arrow->getTargetItem()->getOwner(), QString(), true);
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
	card->setColor(event->getColor());
	card->setPT(event->getPt());
	card->setAnnotation(event->getAnnotation());
	card->setDestroyOnZoneChange(event->getDestroyOnZoneChange());

	emit logCreateToken(this, card->getName(), card->getPT());
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

void Player::eventSetCardCounter(Event_SetCardCounter *event)
{
	CardZone *zone = zones.value(event->getZone(), 0);
	if (!zone)
		return;

	CardItem *card = zone->getCard(event->getCardId(), QString());
	if (!card)
		return;
	
	int oldValue = card->getCounters().value(event->getCounterId(), 0);
	card->setCounter(event->getCounterId(), event->getCounterValue());
	emit logSetCardCounter(this, card->getName(), event->getCounterId(), event->getCounterValue(), oldValue);
}

void Player::eventCreateCounters(Event_CreateCounters *event)
{
	const QList<ServerInfo_Counter *> &eventCounterList = event->getCounterList();
	for (int i = 0; i < eventCounterList.size(); ++i)
		addCounter(eventCounterList[i]);
}

void Player::eventSetCounter(Event_SetCounter *event)
{
	AbstractCounter *c = counters.value(event->getCounterId(), 0);
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
	Player *targetPlayer = static_cast<TabGame *>(parent())->getPlayers().value(event->getTargetPlayerId());
	if (!targetPlayer)
		return;
	CardZone *targetZone = targetPlayer->getZones().value(event->getTargetZone(), 0);
	if (!startZone || !targetZone)
		return;
	
	int position = event->getPosition();
	int x = event->getX();
	int y = event->getY();

	int logPosition = position;
	int logX = x;
	if (x == -1)
		x = 0;
	CardItem *card = startZone->takeCard(position, event->getCardId(), startZone != targetZone);
	if (!card)
		return;
	card->setName(event->getCardName());
	
	if (card->getAttachedTo() && (startZone != targetZone)) {
		CardItem *parentCard = card->getAttachedTo();
		card->setAttachedTo(0);
		parentCard->getZone()->reorganizeCards();
	}

	card->deleteDragItem();

	card->setId(event->getNewCardId());
	card->setFaceDown(event->getFaceDown());
	if (startZone != targetZone) {
		card->setBeingPointedAt(false);
		card->setHovered(false);
		
		const QList<CardItem *> &attachedCards = card->getAttachedCards();
		for (int i = 0; i < attachedCards.size(); ++i)
			attachedCards[i]->setParentItem(targetZone);
		
		if (startZone->getPlayer() != targetZone->getPlayer())
			card->setOwner(targetZone->getPlayer());
	}

	// The log event has to be sent before the card is added to the target zone
	// because the addCard function can modify the card object.
	emit logMoveCard(this, card->getName(), startZone, logPosition, targetZone, logX);

	targetZone->addCard(card, true, x, y);

	// Look at all arrows from and to the card.
	// If the card was moved to another zone, delete the arrows, otherwise update them.
	QMapIterator<int, Player *> playerIterator(static_cast<TabGame *>(parent())->getPlayers());
	while (playerIterator.hasNext()) {
		Player *p = playerIterator.next().value();

		QList<ArrowItem *> arrowsToDelete;
		QMapIterator<int, ArrowItem *> arrowIterator(p->getArrows());
		while (arrowIterator.hasNext()) {
			ArrowItem *arrow = arrowIterator.next().value();
			if ((arrow->getStartItem() == card) || (arrow->getTargetItem() == card)) {
				if (startZone == targetZone)
					arrow->updatePath();
				else
					arrowsToDelete.append(arrow);
			}
		}
		for (int i = 0; i < arrowsToDelete.size(); ++i)
			arrowsToDelete[i]->delArrow();
	}
}

void Player::eventFlipCard(Event_FlipCard *event)
{
	CardZone *zone = zones.value(event->getZone(), 0);
	if (!zone)
		return;
	CardItem *card = zone->getCard(event->getCardId(), event->getCardName());
	if (!card)
		return;
	emit logFlipCard(this, card->getName(), event->getFaceDown());
	card->setFaceDown(event->getFaceDown());
}

void Player::eventDestroyCard(Event_DestroyCard *event)
{
	CardZone *zone = zones.value(event->getZone(), 0);
	if (!zone)
		return;
	
	CardItem *card = zone->getCard(event->getCardId(), QString());
	if (!card)
		return;
	
	emit logDestroyCard(this, card->getName());
	zone->takeCard(-1, event->getCardId(), true);
	card->deleteLater();
}

void Player::eventAttachCard(Event_AttachCard *event)
{
	const QMap<int, Player *> &playerList = static_cast<TabGame *>(parent())->getPlayers();
	int targetPlayerId = event->getTargetPlayerId();
	Player *targetPlayer = 0;
	CardZone *targetZone = 0;
	CardItem *targetCard = 0;
	if (targetPlayerId != -1)
		targetPlayer = playerList.value(targetPlayerId, 0);
	if (targetPlayer)
		targetZone = targetPlayer->getZones().value(event->getTargetZone(), 0);
	if (targetZone)
		targetCard = targetZone->getCard(event->getTargetCardId(), QString());
	
	CardZone *startZone = getZones().value(event->getStartZone(), 0);
	if (!startZone)
		return;
	
	CardItem *startCard = startZone->getCard(event->getCardId(), QString());
	if (!startCard)
		return;
	
	CardItem *oldParent = startCard->getAttachedTo();
	
	startCard->setAttachedTo(targetCard);
	
	startZone->reorganizeCards();
	if ((startZone != targetZone) && targetZone)
		targetZone->reorganizeCards();
	if (oldParent)
		oldParent->getZone()->reorganizeCards();
	
	if (targetCard)
		emit logAttachCard(this, startCard->getName(), targetPlayer, targetCard->getName());
	else
		emit logUnattachCard(this, startCard->getName());
}

void Player::eventDrawCards(Event_DrawCards *event)
{
	CardZone *deck = zones.value("deck");
	CardZone *hand = zones.value("hand");
	const QList<ServerInfo_Card *> &cardList = event->getCardList();
	if (!cardList.isEmpty())
		for (int i = 0; i < cardList.size(); ++i) {
			CardItem *card = deck->takeCard(0, cardList[i]->getId());
			card->setName(cardList[i]->getName());
			hand->addCard(card, false, -1);
		}
	else
		for (int i = 0; i < event->getNumberCards(); ++i)
			hand->addCard(deck->takeCard(0, -1), false, -1);
	
	hand->reorganizeCards();
	deck->reorganizeCards();
	
	emit logDrawCards(this, event->getNumberCards());
}

void Player::eventRevealCards(Event_RevealCards *event)
{
	CardZone *zone = zones.value(event->getZoneName());
	if (!zone)
		return;
	Player *otherPlayer = 0;
	if (event->getOtherPlayerId() != -1) {
		otherPlayer = static_cast<TabGame *>(parent())->getPlayers().value(event->getOtherPlayerId());
		if (!otherPlayer)
			return;
	}
	
	QList<ServerInfo_Card *> cardList = event->getCardList();
	if (!cardList.isEmpty())
		static_cast<GameScene *>(scene())->addRevealedZoneView(this, zone, cardList);
	
	QString cardName;
	if (cardList.size() == 1)
		cardName = cardList.first()->getName();
	emit logRevealCards(this, zone, event->getCardId(), cardName, otherPlayer);
}

void Player::processGameEvent(GameEvent *event, GameEventContext *context)
{
	qDebug() << "player event: id=" << event->getItemId();
	switch (event->getItemId()) {
		case ItemId_Event_Say: eventSay(qobject_cast<Event_Say *>(event)); break;
		case ItemId_Event_Shuffle: eventShuffle(qobject_cast<Event_Shuffle *>(event)); break;
		case ItemId_Event_RollDie: eventRollDie(qobject_cast<Event_RollDie *>(event)); break;
		case ItemId_Event_CreateArrows: eventCreateArrows(qobject_cast<Event_CreateArrows *>(event)); break;
		case ItemId_Event_DeleteArrow: eventDeleteArrow(qobject_cast<Event_DeleteArrow *>(event)); break;
		case ItemId_Event_CreateToken: eventCreateToken(qobject_cast<Event_CreateToken *>(event)); break;
		case ItemId_Event_SetCardAttr: eventSetCardAttr(qobject_cast<Event_SetCardAttr *>(event)); break;
		case ItemId_Event_SetCardCounter: eventSetCardCounter(qobject_cast<Event_SetCardCounter *>(event)); break;
		case ItemId_Event_CreateCounters: eventCreateCounters(qobject_cast<Event_CreateCounters *>(event)); break;
		case ItemId_Event_SetCounter: eventSetCounter(qobject_cast<Event_SetCounter *>(event)); break;
		case ItemId_Event_DelCounter: eventDelCounter(qobject_cast<Event_DelCounter *>(event)); break;
		case ItemId_Event_DumpZone: eventDumpZone(qobject_cast<Event_DumpZone *>(event)); break;
		case ItemId_Event_StopDumpZone: eventStopDumpZone(qobject_cast<Event_StopDumpZone *>(event)); break;
		case ItemId_Event_MoveCard: eventMoveCard(qobject_cast<Event_MoveCard *>(event)); break;
		case ItemId_Event_FlipCard: eventFlipCard(qobject_cast<Event_FlipCard *>(event)); break;
		case ItemId_Event_DestroyCard: eventDestroyCard(qobject_cast<Event_DestroyCard *>(event)); break;
		case ItemId_Event_AttachCard: eventAttachCard(qobject_cast<Event_AttachCard *>(event)); break;
		case ItemId_Event_DrawCards: eventDrawCards(qobject_cast<Event_DrawCards *>(event)); break;
		case ItemId_Event_RevealCards: eventRevealCards(qobject_cast<Event_RevealCards *>(event)); break;
		default: {
			qDebug() << "unhandled game event";
		}
	}
}

void Player::setActive(bool _active)
{
	active = _active;
	table->setActive(active);
	update();
}

QRectF Player::boundingRect() const
{
	return bRect;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	int totalWidth = CARD_HEIGHT + counterAreaWidth + 15;
	if (bgPixmap.isNull())
		painter->fillRect(QRectF(0, 0, totalWidth, boundingRect().height()), QColor(200, 200, 200));
	else
		painter->fillRect(QRectF(0, 0, totalWidth, boundingRect().height()), QBrush(bgPixmap));
}

void Player::processPlayerInfo(ServerInfo_Player *info)
{
	clearCounters();
	clearArrows();
	
	QMapIterator<QString, CardZone *> zoneIt(zones);
	while (zoneIt.hasNext())
		zoneIt.next().value()->clearContents();

	QList<ServerInfo_Zone *> zl = info->getZoneList();
	for (int i = 0; i < zl.size(); ++i) {
		ServerInfo_Zone *zoneInfo = zl[i];
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

	QList<ServerInfo_Counter *> cl = info->getCounterList();
	for (int i = 0; i < cl.size(); ++i) {
		addCounter(cl.at(i));
	}

	QList<ServerInfo_Arrow *> al = info->getArrowList();
	for (int i = 0; i < al.size(); ++i)
		addArrow(al.at(i));
}

void Player::processCardAttachment(ServerInfo_Player *info)
{
	QList<ServerInfo_Zone *> zl = info->getZoneList();
	for (int i = 0; i < zl.size(); ++i) {
		ServerInfo_Zone *zoneInfo = zl[i];
		CardZone *zone = zones.value(zoneInfo->getName(), 0);
		if (!zone)
			continue;

		const QList<ServerInfo_Card *> &cardList = zoneInfo->getCardList();
		for (int j = 0; j < cardList.size(); ++j) {
			ServerInfo_Card *cardInfo = cardList[j];
			if (cardInfo->getAttachPlayerId() != -1) {
				CardItem *startCard = zone->getCard(cardInfo->getId(), QString());
				CardItem *targetCard = static_cast<TabGame *>(parent())->getCard(cardInfo->getAttachPlayerId(), cardInfo->getAttachZone(), cardInfo->getAttachCardId());
				if (!targetCard)
					continue;
				
				startCard->setAttachedTo(targetCard);
			}
		}
	}
}

void Player::playCard(CardItem *c, bool faceDown, bool tapped)
{
	CardInfo *ci = c->getInfo();
	if (ci->getTableRow() == 3)
		sendGameCommand(new Command_MoveCard(-1, c->getZone()->getName(), QList<CardId *>() << new CardId(c->getId()), getId(), "stack", 0, 0, false));
	else {
		QPoint gridPoint = QPoint(-1, 2 - ci->getTableRow());
		sendGameCommand(new Command_MoveCard(-1, c->getZone()->getName(), QList<CardId *>() << new CardId(c->getId()), getId(), "table", gridPoint.x(), gridPoint.y(), faceDown, tapped));
	}
}

void Player::addCard(CardItem *c)
{
	emit newCardAdded(c);
}

void Player::deleteCard(CardItem *c)
{
	if (dialogSemaphore)
		cardsToDelete.append(c);
	else
		c->deleteLater();
}

void Player::addZone(CardZone *z)
{
	zones.insert(z->getName(), z);
}

AbstractCounter *Player::addCounter(ServerInfo_Counter *counter)
{
	return addCounter(counter->getId(), counter->getName(), counter->getColor().getQColor(), counter->getRadius(), counter->getCount());
}

AbstractCounter *Player::addCounter(int counterId, const QString &name, QColor color, int radius, int value)
{
	qDebug() << "addCounter:" << getName() << counterId << name;
	if (counters.contains(counterId))
		return 0;
	
	AbstractCounter *c;
	if (name == "life")
		c = playerTarget->addCounter(counterId, name, value);
	else
		c = new GeneralCounter(this, counterId, name, color, radius, value, this);
	counters.insert(counterId, c);
	if (countersMenu)
		countersMenu->addMenu(c->getMenu());
	if (shortcutsActive)
		c->setShortcutsActive();
	rearrangeCounters();
	return c;
}

void Player::delCounter(int counterId)
{
	AbstractCounter *c = counters.value(counterId, 0);
	if (!c)
		return;
	if (c->getName() == "life")
		playerTarget->delCounter();
	counters.remove(counterId);
	c->delCounter();
	rearrangeCounters();
}

void Player::clearCounters()
{
	QMapIterator<int, AbstractCounter *> counterIterator(counters);
	while (counterIterator.hasNext())
		counterIterator.next().value()->delCounter();
	counters.clear();
	playerTarget->delCounter();
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
	if (!startZone || (!targetZone && !arrow->getTargetZone().isEmpty()))
		return 0;
	
	CardItem *startCard = startZone->getCard(arrow->getStartCardId(), QString());
	CardItem *targetCard = 0;
	if (targetZone)
		targetCard = targetZone->getCard(arrow->getTargetCardId(), QString());
	if (!startCard || (!targetCard && !arrow->getTargetZone().isEmpty()))
		return 0;
	
	if (targetCard)
		return addArrow(arrow->getId(), startCard, targetCard, arrow->getColor().getQColor());
	else
		return addArrow(arrow->getId(), startCard, targetPlayer->getPlayerTarget(), arrow->getColor().getQColor());
}

ArrowItem *Player::addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color)
{
	ArrowItem *arrow = new ArrowItem(this, arrowId, startCard, targetItem, color);
	arrows.insert(arrowId, arrow);
	scene()->addItem(arrow);
	return arrow;
}

void Player::delArrow(int arrowId)
{
	ArrowItem *a = arrows.value(arrowId, 0);
	if (!a)
		return;
	a->delArrow();
}

void Player::removeArrow(ArrowItem *arrow)
{
	if (arrow->getId() != -1)
		arrows.remove(arrow->getId());
}

void Player::clearArrows()
{
	QMapIterator<int, ArrowItem *> arrowIterator(arrows);
	while (arrowIterator.hasNext())
		arrowIterator.next().value()->delArrow();
	arrows.clear();
}

void Player::rearrangeCounters()
{
	qreal marginTop = 80;
	
	// Determine total height of bounding rectangles
	qreal totalHeight = 0;
	QMapIterator<int, AbstractCounter *> counterIterator(counters);
	while (counterIterator.hasNext()) {
		counterIterator.next();
		if (counterIterator.value()->getShownInCounterArea())
			totalHeight += counterIterator.value()->boundingRect().height();
	}
	
	const qreal padding = 10;
	qreal y = boundingRect().y() + marginTop;
	
	// Place objects
	for (counterIterator.toFront(); counterIterator.hasNext(); ) {
		AbstractCounter *c = counterIterator.next().value();

		if (!c->getShownInCounterArea())
			continue;
		
		QRectF br = c->boundingRect();
		c->setPos((counterAreaWidth - br.width()) / 2, y);
		y += br.height() + padding;
	}
}

void Player::sendGameCommand(GameCommand *command)
{
	static_cast<TabGame *>(parent())->sendGameCommand(command, id);
}

void Player::sendCommandContainer(CommandContainer *cont)
{
	static_cast<TabGame *>(parent())->sendCommandContainer(cont, id);
}

bool Player::clearCardsToDelete()
{
	if (cardsToDelete.isEmpty())
		return false;
	
	for (int i = 0; i < cardsToDelete.size(); ++i)
		cardsToDelete[i]->deleteLater();
	cardsToDelete.clear();
	
	return true;
}

void Player::cardMenuAction(QAction *a)
{
	QList<QGraphicsItem *> sel = scene()->selectedItems();
	QList<CardItem *> cardList;
	while (!sel.isEmpty())
		cardList.append(qgraphicsitem_cast<CardItem *>(sel.takeFirst()));
	
	QList<Command *> commandList;
	if (a->data().toInt() <= 4)
		for (int i = 0; i < cardList.size(); ++i) {
			CardItem *card = cardList[i];
			switch (a->data().toInt()) {
				case 0:
					if (!card->getTapped())
						commandList.append(new Command_SetCardAttr(-1, card->getZone()->getName(), card->getId(), "tapped", "1"));
					break;
				case 1:
					if (card->getTapped())
						commandList.append(new Command_SetCardAttr(-1, card->getZone()->getName(), card->getId(), "tapped", "0"));
					break;
				case 2:
					commandList.append(new Command_SetCardAttr(-1, card->getZone()->getName(), card->getId(), "doesnt_untap", QString::number(!card->getDoesntUntap())));
					break;
				case 3: {
					QString zone = card->getZone()->getName();
					commandList.append(new Command_FlipCard(-1, zone, card->getId(), !card->getFaceDown()));
					break;
				}
				case 4:
					commandList.append(new Command_CreateToken(-1, card->getZone()->getName(), card->getName(), card->getColor(), card->getPT(), card->getAnnotation(), true, -1, card->getGridPoint().y()));
					break;
			}
		}
	else {
		QList<CardId *> idList;
		for (int i = 0; i < cardList.size(); ++i)
			idList.append(new CardId(cardList[i]->getId()));
		QString startZone = cardList[0]->getZone()->getName();
		
		switch (a->data().toInt()) {
			case 5:
				commandList.append(new Command_MoveCard(-1, startZone, idList, getId(), "deck", 0, 0, false));
				break;
			case 6:
				commandList.append(new Command_MoveCard(-1, startZone, idList, getId(), "deck", -1, 0, false));
				break;
			case 7:
				commandList.append(new Command_MoveCard(-1, startZone, idList, getId(), "grave", 0, 0, false));
				break;
			case 8:
				commandList.append(new Command_MoveCard(-1, startZone, idList, getId(), "rfg", 0, 0, false));
				break;
			default: ;
		}
	}
	sendCommandContainer(new CommandContainer(commandList));
}

void Player::actSetPT(QAction * /*a*/)
{
	QString oldPT;
	QListIterator<QGraphicsItem *> i(scene()->selectedItems());
	while (i.hasNext()) {
		CardItem *card = static_cast<CardItem *>(i.next());
		if (!card->getPT().isEmpty())
			oldPT = card->getPT();
	}
	bool ok;
	dialogSemaphore = true;
	QString pt = QInputDialog::getText(0, tr("Set power/toughness"), tr("Please enter the new PT:"), QLineEdit::Normal, oldPT, &ok);
	dialogSemaphore = false;
	if (clearCardsToDelete())
		return;
	if (!ok)
		return;
	
	QListIterator<QGraphicsItem *> j(scene()->selectedItems());
	while (j.hasNext()) {
		CardItem *card = static_cast<CardItem *>(j.next());
		sendGameCommand(new Command_SetCardAttr(-1, card->getZone()->getName(), card->getId(), "pt", pt));
	}
}

void Player::actSetAnnotation(QAction * /*a*/)
{
	QString oldAnnotation;
	QListIterator<QGraphicsItem *> i(scene()->selectedItems());
	while (i.hasNext()) {
		CardItem *card = static_cast<CardItem *>(i.next());
		if (!card->getAnnotation().isEmpty())
			oldAnnotation = card->getAnnotation();
	}
	
	bool ok;
	dialogSemaphore = true;
	QString annotation = QInputDialog::getText(0, tr("Set annotation"), tr("Please enter the new annotation:"), QLineEdit::Normal, oldAnnotation, &ok);
	dialogSemaphore = false;
	if (clearCardsToDelete())
		return;
	if (!ok)
		return;
	
	i.toFront();
	while (i.hasNext()) {
		CardItem *card = static_cast<CardItem *>(i.next());
		sendGameCommand(new Command_SetCardAttr(-1, card->getZone()->getName(), card->getId(), "annotation", annotation));
	}
}

void Player::actAttach(QAction *a)
{
	CardItem *card = static_cast<CardItem *>(a->parent());
	ArrowAttachItem *arrow = new ArrowAttachItem(card);
	scene()->addItem(arrow);
	arrow->grabMouse();
}

void Player::actUnattach(QAction *a)
{
	CardItem *card = static_cast<CardItem *>(a->parent());
	sendGameCommand(new Command_AttachCard(-1, card->getZone()->getName(), card->getId(), -1, QString(), -1));
}

void Player::actCardCounterTrigger(QAction *a)
{
	int counterId = a->data().toInt() / 1000;
	int action = a->data().toInt() % 1000;
	switch (action) {
		case 9: {
			QListIterator<QGraphicsItem *> i(scene()->selectedItems());
			while (i.hasNext()) {
				CardItem *card = static_cast<CardItem *>(i.next());
				if (card->getCounters().value(counterId, 0) < MAX_COUNTERS_ON_CARD)
					sendGameCommand(new Command_SetCardCounter(-1, card->getZone()->getName(), card->getId(), counterId, card->getCounters().value(counterId, 0) + 1));
			}
			break;
		}
		case 10: {
			QListIterator<QGraphicsItem *> i(scene()->selectedItems());
			while (i.hasNext()) {
				CardItem *card = static_cast<CardItem *>(i.next());
				if (card->getCounters().value(counterId, 0))
					sendGameCommand(new Command_SetCardCounter(-1, card->getZone()->getName(), card->getId(), counterId, card->getCounters().value(counterId, 0) - 1));
			}
			break;
		}
		case 11: {
			bool ok;
			dialogSemaphore = true;
			int number = QInputDialog::getInteger(0, tr("Set counters"), tr("Number:"), 0, 0, MAX_COUNTERS_ON_CARD, 1, &ok);
			dialogSemaphore = false;
			if (clearCardsToDelete())
				return;
			if (!ok)
				return;
		
			QListIterator<QGraphicsItem *> i(scene()->selectedItems());
			while (i.hasNext()) {
				CardItem *card = static_cast<CardItem *>(i.next());
				sendGameCommand(new Command_SetCardCounter(-1, card->getZone()->getName(), card->getId(), counterId, number));
			}
			break;
		}
		default: ;
	}
}

void Player::setCardMenu(QMenu *menu)
{
	if (aCardMenu)
		aCardMenu->setMenu(menu);
}

QMenu *Player::getCardMenu() const
{
	if (aCardMenu)
		return aCardMenu->menu();
	return 0;
}

QString Player::getName() const
{
	return userInfo->getName();
}

qreal Player::getMinimumWidth() const
{
	qreal result = table->getMinimumWidth() + CARD_HEIGHT + 15 + counterAreaWidth + stack->boundingRect().width();
	if (!settingsCache->getHorizontalHand())
		result += hand->boundingRect().width();
	return result;
}

void Player::setMirrored(bool _mirrored)
{
	if (mirrored != _mirrored) {
		mirrored = _mirrored;
		rearrangeZones();
	}
}

void Player::processSceneSizeChange(const QSizeF &newSize)
{
	// This will need to be changed if player areas are displayed side by side (e.g. 2x2 for a 4-player game)
	qreal fullPlayerWidth = newSize.width();
	
	qreal tableWidth = fullPlayerWidth - CARD_HEIGHT - 15 - counterAreaWidth - stack->boundingRect().width();
	if (!settingsCache->getHorizontalHand())
		tableWidth -= hand->boundingRect().width();
	
	table->setWidth(tableWidth);
	hand->setWidth(tableWidth + stack->boundingRect().width());
}
