#include "tab_supervisor.h"
#include "client.h"
#include "tab_server.h"
#include "tab_chatchannel.h"
#include "tab_game.h"
#include "protocol_items.h"

TabSupervisor::	TabSupervisor(QWidget *parent)
	: QTabWidget(parent), client(0), tabServer(0)
{

}

void TabSupervisor::retranslateUi()
{
	if (tabServer)
		setTabText(0, tr("Server"));
}

void TabSupervisor::start(Client *_client)
{
	client = _client;
	connect(client, SIGNAL(chatEventReceived(ChatEvent *)), this, SLOT(processChatEvent(ChatEvent *)));
	connect(client, SIGNAL(gameEventReceived(GameEvent *)), this, SLOT(processGameEvent(GameEvent *)));
	connect(client, SIGNAL(gameJoinedEventReceived(Event_GameJoined *)), this, SLOT(gameJoined(Event_GameJoined *)));

	tabServer = new TabServer(client);
	connect(tabServer, SIGNAL(gameJoined(int)), this, SLOT(addGameTab(int)));
	connect(tabServer, SIGNAL(chatChannelJoined(const QString &)), this, SLOT(addChatChannelTab(const QString &)));
	
	addTab(tabServer, QString());

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
	
	QMapIterator<QString, TabChatChannel *> chatChannelIterator(chatChannelTabs);
	while (chatChannelIterator.hasNext())
		delete chatChannelIterator.next().value();
	chatChannelTabs.clear();

	QMapIterator<int, TabGame *> gameIterator(gameTabs);
	while (gameIterator.hasNext())
		delete gameIterator.next().value();
	gameTabs.clear();
}

void TabSupervisor::gameJoined(Event_GameJoined *event)
{
	TabGame *tab = new TabGame(client, event->getGameId());
	addTab(tab, tr("Game %1").arg(event->getGameId()));
	gameTabs.insert(event->getGameId(), tab);
}

void TabSupervisor::addChatChannelTab(const QString &channelName)
{
	TabChatChannel *tab = new TabChatChannel(client, channelName);
	addTab(tab, channelName);
	chatChannelTabs.insert(channelName, tab);
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
	if (tab)
		tab->processGameEvent(event);
}
