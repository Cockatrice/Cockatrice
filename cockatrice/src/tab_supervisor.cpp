#include "tab_supervisor.h"
#include "client.h"
#include "tab_server.h"
#include "tab_chatchannel.h"
#include "tab_game.h"
#include "tab_deck_storage.h"
#include "protocol_items.h"
#include "pingpixmapgenerator.h"

TabSupervisor::	TabSupervisor(QWidget *parent)
	: QTabWidget(parent), client(0), tabServer(0), tabDeckStorage(0)
{
	setIconSize(QSize(15, 15));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(updateMenu(int)));
}

void TabSupervisor::retranslateUi()
{
	if (tabServer) {
		setTabText(0, tr("Server"));
		tabServer->retranslateUi();
	}
	if (tabDeckStorage) {
		setTabText(1, tr("Deck storage"));
		tabDeckStorage->retranslateUi();
	}
	
	QMapIterator<QString, TabChatChannel *> chatChannelIterator(chatChannelTabs);
	while (chatChannelIterator.hasNext())
		chatChannelIterator.next().value()->retranslateUi();

	QMapIterator<int, TabGame *> gameIterator(gameTabs);
	while (gameIterator.hasNext())
		gameIterator.next().value()->retranslateUi();
}

void TabSupervisor::start(Client *_client)
{
	client = _client;
	connect(client, SIGNAL(chatEventReceived(ChatEvent *)), this, SLOT(processChatEvent(ChatEvent *)));
	connect(client, SIGNAL(gameEventReceived(GameEvent *)), this, SLOT(processGameEvent(GameEvent *)));
	connect(client, SIGNAL(gameJoinedEventReceived(Event_GameJoined *)), this, SLOT(gameJoined(Event_GameJoined *)));
	connect(client, SIGNAL(maxPingTime(int, int)), this, SLOT(updatePingTime(int, int)));

	tabServer = new TabServer(client);
	connect(tabServer, SIGNAL(chatChannelJoined(const QString &)), this, SLOT(addChatChannelTab(const QString &)));
	addTab(tabServer, QString());
	updatePingTime(0, -1);
	
	tabDeckStorage = new TabDeckStorage(client);
	addTab(tabDeckStorage, QString());

	retranslateUi();
}

void TabSupervisor::stop()
{
	if (!client)
		return;
	
	disconnect(client, 0, this, 0);
	
	clear();
	
	delete tabServer;
	tabServer = 0;
	
	delete tabDeckStorage;
	tabDeckStorage = 0;
	
	QMapIterator<QString, TabChatChannel *> chatChannelIterator(chatChannelTabs);
	while (chatChannelIterator.hasNext())
		delete chatChannelIterator.next().value();
	chatChannelTabs.clear();

	QMapIterator<int, TabGame *> gameIterator(gameTabs);
	while (gameIterator.hasNext())
		delete gameIterator.next().value();
	gameTabs.clear();
}

void TabSupervisor::updatePingTime(int value, int max)
{
	if (!tabServer)
		return;
	
	setTabIcon(0, QIcon(PingPixmapGenerator::generatePixmap(15, value, max)));
}

void TabSupervisor::gameJoined(Event_GameJoined *event)
{
	TabGame *tab = new TabGame(client, event->getGameId(), event->getPlayerId(), event->getSpectator(), event->getResuming());
	connect(tab, SIGNAL(gameClosing(TabGame *)), this, SLOT(gameLeft(TabGame *)));
	addTab(tab, tr("Game %1").arg(event->getGameId()));
	gameTabs.insert(event->getGameId(), tab);
	setCurrentWidget(tab);
}

void TabSupervisor::gameLeft(TabGame *tab)
{
	emit setMenu(0);

	gameTabs.remove(tab->getGameId());
	removeTab(indexOf(tab));
}

void TabSupervisor::addChatChannelTab(const QString &channelName)
{
	TabChatChannel *tab = new TabChatChannel(client, channelName);
	connect(tab, SIGNAL(channelClosing(TabChatChannel *)), this, SLOT(chatChannelLeft(TabChatChannel *)));
	addTab(tab, channelName);
	chatChannelTabs.insert(channelName, tab);
	setCurrentWidget(tab);
}

void TabSupervisor::chatChannelLeft(TabChatChannel *tab)
{
	emit setMenu(0);

	chatChannelTabs.remove(tab->getChannelName());
	removeTab(indexOf(tab));
}

void TabSupervisor::processChatEvent(ChatEvent *event)
{
	TabChatChannel *tab = chatChannelTabs.value(event->getChannel(), 0);
	if (tab)
		tab->processChatEvent(event);
}

void TabSupervisor::processGameEvent(GameEvent *event)
{
	TabGame *tab = gameTabs.value(event->getGameId());
	if (tab) {
		qDebug() << "gameEvent gameId =" << event->getGameId();
		tab->processGameEvent(event);
	} else
		qDebug() << "gameEvent: invalid gameId";
}

void TabSupervisor::updateMenu(int index)
{
	if (index != -1)
		emit setMenu(static_cast<Tab *>(widget(index))->getTabMenu());
	else
		emit setMenu(0);
}
