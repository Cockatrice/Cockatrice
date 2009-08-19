#include <QGraphicsScene>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <stdlib.h>
#include "serverplayer.h"
#include "game.h"
#include "servereventdata.h"
#include "client.h"
#include "tablezone.h"
#include "handzone.h"
#include "carddatabase.h"
#include "dlg_startgame.h"
#include "playerarea.h"
#include "counter.h"

Game::Game(CardDatabase *_db, Client *_client, QGraphicsScene *_scene, QMenu *_actionsMenu, QMenu *_cardMenu, int playerId, const QString &playerName, QObject *parent)
	: QObject(parent), actionsMenu(_actionsMenu), cardMenu(_cardMenu), db(_db), client(_client), scene(_scene), started(false), currentPhase(-1)
{
	QRectF sr = scene->sceneRect();
	localPlayer = addPlayer(playerId, playerName, QPointF(0, sr.y() + sr.height() / 2 + 2), true);

	connect(client, SIGNAL(gameEvent(const ServerEventData &)), this, SLOT(gameEvent(const ServerEventData &)));
	connect(client, SIGNAL(playerListReceived(QList<ServerPlayer *>)), this, SLOT(playerListReceived(QList<ServerPlayer *>)));

	aUntapAll = new QAction(tr("&Untap all permanents"), this);
	aUntapAll->setShortcut(tr("Ctrl+U"));
	connect(aUntapAll, SIGNAL(triggered()), this, SLOT(actUntapAll()));

	aDecLife = new QAction(tr("&Decrement life"), this);
	aDecLife->setShortcut(tr("F11"));
	connect(aDecLife, SIGNAL(triggered()), this, SLOT(actDecLife()));
	aIncLife = new QAction(tr("&Increment life"), this);
	aIncLife->setShortcut(tr("F12"));
	connect(aIncLife, SIGNAL(triggered()), this, SLOT(actIncLife()));
	aSetLife = new QAction(tr("&Set life"), this);
	aSetLife->setShortcut(tr("Ctrl+L"));
	connect(aSetLife, SIGNAL(triggered()), this, SLOT(actSetLife()));

	aShuffle = new QAction(tr("&Shuffle"), this);
	aShuffle->setShortcut(tr("Ctrl+S"));
	connect(aShuffle, SIGNAL(triggered()), this, SLOT(actShuffle()));
	aDraw = new QAction(tr("&Draw a card"), this);
	aDraw->setShortcut(tr("Ctrl+D"));
	connect(aDraw, SIGNAL(triggered()), this, SLOT(actDrawCard()));
	aDrawCards = new QAction(tr("D&raw cards..."), this);
	connect(aDrawCards, SIGNAL(triggered()), this, SLOT(actDrawCards()));
	aDrawCards->setShortcut(tr("Ctrl+E"));
	aRollDice = new QAction(tr("R&oll dice..."), this);
	aRollDice->setShortcut(tr("Ctrl+I"));
	connect(aRollDice, SIGNAL(triggered()), this, SLOT(actRollDice()));

	aCreateToken = new QAction(tr("&Create token..."), this);
	aCreateToken->setShortcut(tr("Ctrl+T"));
	connect(aCreateToken, SIGNAL(triggered()), this, SLOT(actCreateToken()));
	
	aNextPhase = new QAction(tr("Next &phase"), this);
	aNextPhase->setShortcut(tr("Ctrl+Space"));
	connect(aNextPhase, SIGNAL(triggered()), this, SLOT(actNextPhase()));
	aNextTurn = new QAction(tr("Next &turn"), this);
	aNextTurn->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Return")) << QKeySequence(tr("Ctrl+Enter")));
	connect(aNextTurn, SIGNAL(triggered()), this, SLOT(actNextTurn()));
	
	actionsMenu->addAction(aNextPhase);
	actionsMenu->addAction(aNextTurn);
	actionsMenu->addSeparator();
	actionsMenu->addAction(aUntapAll);
	actionsMenu->addSeparator();
	actionsMenu->addAction(aDecLife);
	actionsMenu->addAction(aIncLife);
	actionsMenu->addAction(aSetLife);
	actionsMenu->addSeparator();
	actionsMenu->addAction(aShuffle);
	actionsMenu->addAction(aDraw);
	actionsMenu->addAction(aDrawCards);
	actionsMenu->addAction(aRollDice);
	actionsMenu->addSeparator();
	actionsMenu->addAction(aCreateToken);
	actionsMenu->addSeparator();
	sayMenu = actionsMenu->addMenu(tr("S&ay"));
	initSayMenu();

	aTap = new QAction(tr("&Tap"), this);
	aUntap = new QAction(tr("&Untap"), this);
	aDoesntUntap = new QAction(tr("Toggle &normal untapping"), this);
	aFlip = new QAction(tr("&Flip"), this);
	aAddCounter = new QAction(tr("&Add counter"), this);
	aRemoveCounter = new QAction(tr("&Remove counter"), this);
	aSetCounters = new QAction(tr("&Set counters..."), this);
	connect(aSetCounters, SIGNAL(triggered()), this, SLOT(actSetCounters()));
	aMoveToTopLibrary = new QAction(tr("&top of library"), this);
	aMoveToBottomLibrary = new QAction(tr("&bottom of library"), this);
	aMoveToGraveyard = new QAction(tr("&graveyard"), this);
	aMoveToExile = new QAction(tr("&exile"), this);

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
	moveMenu = cardMenu->addMenu(tr("&Move to"));
	
	moveMenu->addAction(aMoveToTopLibrary);
	moveMenu->addAction(aMoveToBottomLibrary);
	moveMenu->addAction(aMoveToGraveyard);
	moveMenu->addAction(aMoveToExile);
	
	cardMenuHandlers.insert(aTap, &Game::actTap);
	cardMenuHandlers.insert(aUntap, &Game::actUntap);
	cardMenuHandlers.insert(aDoesntUntap, &Game::actDoesntUntap);
	cardMenuHandlers.insert(aFlip, &Game::actFlip);
	cardMenuHandlers.insert(aAddCounter, &Game::actAddCounter);
	cardMenuHandlers.insert(aRemoveCounter, &Game::actRemoveCounter);
	cardMenuHandlers.insert(aMoveToTopLibrary, &Game::actMoveToTopLibrary);
	cardMenuHandlers.insert(aMoveToBottomLibrary, &Game::actMoveToBottomLibrary);
	cardMenuHandlers.insert(aMoveToGraveyard, &Game::actMoveToGraveyard);
	cardMenuHandlers.insert(aMoveToExile, &Game::actMoveToExile);
	
	QHashIterator<QAction *, CardMenuHandler> i(cardMenuHandlers);
	while (i.hasNext()) {
		i.next();
		connect(i.key(), SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	}

	dlgStartGame = new DlgStartGame(db);
	connect(dlgStartGame, SIGNAL(newDeckLoaded(const QStringList &)), client, SLOT(submitDeck(const QStringList &)));
	connect(dlgStartGame, SIGNAL(finished(int)), this, SLOT(readyStart()));
}

Game::~Game()
{
	qDebug("Game destructor");
	for (int i = 0; i < players.size(); i++) {
		emit playerRemoved(players.at(i));
		delete players.at(i);
	}
}

void Game::initSayMenu()
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

Player *Game::addPlayer(int playerId, const QString &playerName, QPointF base, bool local)
{
	Player *newPlayer = new Player(playerName, playerId, base, local, db, client, scene, this);

	connect(newPlayer, SIGNAL(sigShowCardMenu(QPoint)), this, SLOT(showCardMenu(QPoint)));
	connect(newPlayer, SIGNAL(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)), this, SIGNAL(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)));
	connect(newPlayer, SIGNAL(logCreateToken(Player *, QString)), this, SIGNAL(logCreateToken(Player *, QString)));
	connect(newPlayer, SIGNAL(logSetCardCounters(Player *, QString, int, int)), this, SIGNAL(logSetCardCounters(Player *, QString, int, int)));
	connect(newPlayer, SIGNAL(logSetTapped(Player *, QString, bool)), this, SIGNAL(logSetTapped(Player *, QString, bool)));
	connect(newPlayer, SIGNAL(logSetCounter(Player *, QString, int, int)), this, SIGNAL(logSetCounter(Player *, QString, int, int)));
	connect(newPlayer, SIGNAL(logSetDoesntUntap(Player *, QString, bool)), this, SIGNAL(logSetDoesntUntap(Player *, QString, bool)));

	players << newPlayer;
	emit playerAdded(newPlayer);

	return newPlayer;
}

void Game::playerListReceived(QList<ServerPlayer *> playerList)
{
	QListIterator<ServerPlayer *> i(playerList);
	QStringList nameList;
	while (i.hasNext()) {
		ServerPlayer *temp = i.next();
		nameList << temp->getName();
		int id = temp->getPlayerId();

		if (id != localPlayer->getId())
			addPlayer(id, temp->getName(), QPointF(0, 0), false);

		delete temp;
	}
	emit logPlayerListReceived(nameList);
	restartGameDialog();
}

void Game::readyStart()
{
	client->readyStart();
}

void Game::restartGameDialog()
{
	dlgStartGame->show();
}

void Game::gameEvent(const ServerEventData &msg)
{
	qDebug(QString("game::gameEvent: public=%1, player=%2, name=%3, type=%4, data=%5").arg(msg.getPublic()).arg(msg.getPlayerId()).arg(msg.getPlayerName()).arg(msg.getEventType()).arg(msg.getEventData().join("/")).toLatin1());
	if (!msg.getPublic())
		localPlayer->gameEvent(msg);
	else {
		Player *p = players.findPlayer(msg.getPlayerId());
		if ((!p) && (msg.getEventType() != eventJoin)) {
			// XXX
		}

		switch(msg.getEventType()) {
		case eventSay:
			emit logSay(p, msg.getEventData()[0]);
			break;
		case eventJoin: {
			Player *newPlayer = addPlayer(msg.getPlayerId(), msg.getPlayerName(), QPointF(0, 0), false);
			emit logJoin(newPlayer);
			break;
		}
		case eventLeave:
			emit logLeave(p);
			// XXX Spieler natürlich noch rauswerfen
			break;
		case eventReadyStart:
			if (started) {
				started = false;
				emit logReadyStart(p);
				if (!p->getLocal())
					restartGameDialog();
			}
			break;
		case eventGameStart:
			started = true;
			emit logGameStart();
			break;
		case eventShuffle:
			emit logShuffle(p);
			break;
		case eventRollDice: {
			QStringList data = msg.getEventData();
			int sides = data[0].toInt();
			int roll = data[1].toInt();
			emit logRollDice(p, sides, roll);
			break;
		}
		case eventSetActivePlayer: {
			QStringList data = msg.getEventData();
			int playerId = data[0].toInt();
			Player *player = players.findPlayer(playerId);
			if (!player) {
				qDebug(QString("setActivePlayer: invalid player: %1").arg(playerId).toLatin1());
				break;
			}
			for (int i = 0; i < players.size(); ++i)
				players[i]->setActive(players[i] == player);
			emit logSetActivePlayer(player);
			break;
		}
		case eventSetActivePhase: {
			QStringList data = msg.getEventData();
			int phase = data[0].toInt();
			currentPhase = phase;
			emit setActivePhase(phase);
			break;
		}

		case eventName:
		case eventCreateToken:
		case eventSetupZones:
		case eventSetCardAttr:
		case eventAddCounter:
		case eventSetCounter:
		case eventDelCounter:
		case eventPlayerId: {
			p->gameEvent(msg);
			break;
		}
		case eventDumpZone: {
			QStringList data = msg.getEventData();
			emit logDumpZone(p, data[1], players.findPlayer(data[0].toInt())->getName(), data[2].toInt());
			break;
		}
		case eventStopDumpZone: {
			QStringList data = msg.getEventData();
			emit logStopDumpZone(p, data[1], players.findPlayer(data[0].toInt())->getName());
			break;
		}
		case eventMoveCard: {
			if (msg.getPlayerId() == localPlayer->getId())
				break;
			p->gameEvent(msg);
			break;
		}
		case eventDraw: {
			emit logDraw(p, msg.getEventData()[0].toInt());
			if (msg.getPlayerId() == localPlayer->getId())
				break;
			p->gameEvent(msg);
			break;
		}
		case eventInvalid:
			qDebug("Unhandled global event");
		}
	}
}

void Game::actNextPhase()
{
	int phase = currentPhase;
	if (++phase >= phaseCount)
		phase = 0;
	client->setActivePhase(phase);
}

void Game::actNextTurn()
{
	client->nextTurn();
}

void Game::actUntapAll()
{
	client->setCardAttr("table", -1, "tapped", "false");
}

void Game::actIncLife()
{
	client->incCounter("life", 1);
}

void Game::actDecLife()
{
	client->incCounter("life", -1);
}

void Game::actSetLife()
{
	bool ok;
	int life = QInputDialog::getInteger(0, tr("Set life"), tr("New life total:"), localPlayer->area->getCounter("life")->getValue(), 0, 2000000000, 1, &ok);
	if (ok)
		client->setCounter("life", life);
}

void Game::actShuffle()
{
	client->shuffle();
}

void Game::actRollDice()
{
	bool ok;
	int sides = QInputDialog::getInteger(0, tr("Roll dice"), tr("Number of sides:"), 20, 2, 1000, 1, &ok);
	if (ok)
		client->rollDice(sides);
}

void Game::actDrawCard()
{
	client->drawCards(1);
}

void Game::actDrawCards()
{
	int number = QInputDialog::getInteger(0, tr("Draw cards"), tr("Number:"));
	if (number)
		client->drawCards(number);
}

void Game::actCreateToken()
{
	QString cardname = QInputDialog::getText(0, tr("Create token"), tr("Name:"));
	client->createToken("table", cardname, QString(), 0, 0);
}

void Game::showCardMenu(QPoint p)
{
	cardMenu->exec(p);
}

void Game::cardMenuAction()
{
	// Determine the appropriate handler function.
	CardMenuHandler handler = cardMenuHandlers.value(static_cast<QAction *>(sender()));
			
	// The list of selected items is randomly shuffled.
	QList<QGraphicsItem *> sel = scene->selectedItems();
	while (!sel.isEmpty()) {
		unsigned int i = (unsigned int) (((double) sel.size()) * qrand() / (RAND_MAX + 1.0));
		qDebug(QString("%1 items left, i=%2").arg(sel.size()).arg(i).toLatin1());
		CardItem *card = qgraphicsitem_cast<CardItem *>(sel.takeAt(i));
		// For each item, the handler function is called.
		(this->*handler)(card);
	}
}

void Game::actTap(CardItem *card)
{
	if (!card->getTapped())
		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "tapped", "1");
}

void Game::actUntap(CardItem *card)
{
	if (card->getTapped())
		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "tapped", "0");
}

void Game::actDoesntUntap(CardItem *card)
{
	client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "doesnt_untap", QString::number(!card->getDoesntUntap()));
}

void Game::actFlip(CardItem *card)
{
	QString zone = qgraphicsitem_cast<CardZone *>(card->parentItem())->getName();
	client->moveCard(card->getId(), zone, zone, card->getGridPoint().x(), card->getGridPoint().y(), !card->getFaceDown());
}

void Game::actAddCounter(CardItem *card)
{
	if (card->getCounters() < MAX_COUNTERS_ON_CARD)
		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "counters", QString::number(card->getCounters() + 1));
}

void Game::actRemoveCounter(CardItem *card)
{
	if (card->getCounters())
		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "counters", QString::number(card->getCounters() - 1));
}

void Game::actSetCounters()
{
	bool ok;
	int number = QInputDialog::getInteger(0, tr("Set counters"), tr("Number:"), 0, 0, MAX_COUNTERS_ON_CARD, 1, &ok);
	if (!ok)
		return;

	QListIterator<QGraphicsItem *> i(scene->selectedItems());
	while (i.hasNext()) {
		CardItem *temp = (CardItem *) i.next();
		client->setCardAttr(qgraphicsitem_cast<CardZone *>(temp->parentItem())->getName(), temp->getId(), "counters", QString::number(number));
	}
}

void Game::actMoveToTopLibrary(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	client->moveCard(card->getId(), startZone->getName(), "deck", 0, 0, false);
}

void Game::actMoveToBottomLibrary(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	client->moveCard(card->getId(), startZone->getName(), "deck", -1, 0, false);
}

void Game::actMoveToGraveyard(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	client->moveCard(card->getId(), startZone->getName(), "grave", 0, 0, false);
}

void Game::actMoveToExile(CardItem *card)
{
	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
	client->moveCard(card->getId(), startZone->getName(), "rfg", 0, 0, false);
}

void Game::actSayMessage()
{
	QAction *a = qobject_cast<QAction *>(sender());
	client->say(a->text());
}

void Game::hoverCardEvent(CardItem *card)
{
	emit hoverCard(card->getName());
}
