#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <stdlib.h>
#include "game.h"
#include "client.h"
#include "tablezone.h"
#include "handzone.h"
#include "carddatabase.h"
#include "counter.h"
#include "gamescene.h"
#include "player.h"
#include "arrowitem.h"
#include "protocol_datastructures.h"

Game::Game(Client *_client, GameScene *_scene, QMenuBar *menuBar, QObject *parent)
	: QObject(parent), client(_client), scene(_scene), started(false), currentPhase(-1)
{
	connect(client, SIGNAL(gameEvent(const ServerEventData &)), this, SLOT(gameEvent(const ServerEventData &)));

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

	cardMenu = menuBar->addMenu(QString());
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

	retranslateUi();
}

Game::~Game()
{
	qDebug("Game destructor");

	QMapIterator<int, Player *> i(players);
	while (i.hasNext()) {
		i.next();
		emit playerRemoved(i.value());
		delete i.value();
	}
	
	delete gameMenu;
	delete cardMenu;
}

void Game::retranslateUi()
{
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

void Game::restartGameDialog()
{
//	dlgStartGame->show();
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
		CardItem *card = qgraphicsitem_cast<CardItem *>(sel.takeAt(i));
		// For each item, the handler function is called.
		(this->*handler)(card);
	}
}

void Game::actTap(CardItem *card)
{
//	if (!card->getTapped())
//		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "tapped", "1");
}

void Game::actUntap(CardItem *card)
{
//	if (card->getTapped())
//		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "tapped", "0");
}

void Game::actDoesntUntap(CardItem *card)
{
//	client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "doesnt_untap", QString::number(!card->getDoesntUntap()));
}

void Game::actFlip(CardItem *card)
{
//	QString zone = qgraphicsitem_cast<CardZone *>(card->parentItem())->getName();
//	client->moveCard(card->getId(), zone, zone, card->getGridPoint().x(), card->getGridPoint().y(), !card->getFaceDown());
}

void Game::actAddCounter(CardItem *card)
{
//	if (card->getCounters() < MAX_COUNTERS_ON_CARD)
//		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "counters", QString::number(card->getCounters() + 1));
}

void Game::actRemoveCounter(CardItem *card)
{
//	if (card->getCounters())
//		client->setCardAttr(qgraphicsitem_cast<CardZone *>(card->parentItem())->getName(), card->getId(), "counters", QString::number(card->getCounters() - 1));
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
//		client->setCardAttr(qgraphicsitem_cast<CardZone *>(temp->parentItem())->getName(), temp->getId(), "counters", QString::number(number));
	}
}

void Game::actMoveToTopLibrary(CardItem *card)
{
//	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
//	client->moveCard(card->getId(), startZone->getName(), "deck", 0, 0, false);
}

void Game::actMoveToBottomLibrary(CardItem *card)
{
//	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
//	client->moveCard(card->getId(), startZone->getName(), "deck", -1, 0, false);
}

void Game::actMoveToGraveyard(CardItem *card)
{
//	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
//	client->moveCard(card->getId(), startZone->getName(), "grave", 0, 0, false);
}

void Game::actMoveToExile(CardItem *card)
{
//	CardZone *startZone = qgraphicsitem_cast<CardZone *>(card->parentItem());
//	client->moveCard(card->getId(), startZone->getName(), "rfg", 0, 0, false);
}
