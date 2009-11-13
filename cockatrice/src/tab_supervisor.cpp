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

	tabServer = new TabServer(client);
	connect(tabServer, SIGNAL(gameJoined(int)), this, SLOT(addGameTab(int)));
	connect(tabServer, SIGNAL(chatChannelJoined(const QString &)), this, SLOT(addChatChannelTab(const QString &)));
	
	addTab(tabServer, QString());

	retranslateUi();
}

void TabSupervisor::stop()
{

}

void TabSupervisor::addGameTab(int gameId)
{

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

}
