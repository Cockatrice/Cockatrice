#include <QApplication>
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
	tabChangedIcon = new QIcon(":/resources/icon_tab_changed.svg");
	setElideMode(Qt::ElideRight);
	setIconSize(QSize(15, 15));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(updateCurrent(int)));
}

TabSupervisor::~TabSupervisor()
{
	delete tabChangedIcon;
}

void TabSupervisor::retranslateUi()
{
	QList<Tab *> tabs;
	if (tabServer)
		tabs.append(tabServer);
	if (tabDeckStorage)
		tabs.append(tabDeckStorage);
	QMapIterator<QString, TabChatChannel *> chatChannelIterator(chatChannelTabs);
	while (chatChannelIterator.hasNext())
		tabs.append(chatChannelIterator.next().value());
	QMapIterator<int, TabGame *> gameIterator(gameTabs);
	while (gameIterator.hasNext())
		tabs.append(gameIterator.next().value());
	
	for (int i = 0; i < tabs.size(); ++i) {
		setTabText(indexOf(tabs[i]), tabs[i]->getTabText());
		tabs[i]->retranslateUi();
	}
}

void TabSupervisor::myAddTab(Tab *tab)
{
	connect(tab, SIGNAL(userEvent()), this, SLOT(tabUserEvent()));
	addTab(tab, tab->getTabText());
}

void TabSupervisor::start(Client *_client)
{
	client = _client;
	connect(client, SIGNAL(chatEventReceived(ChatEvent *)), this, SLOT(processChatEvent(ChatEvent *)));
	connect(client, SIGNAL(gameEventContainerReceived(GameEventContainer *)), this, SLOT(processGameEventContainer(GameEventContainer *)));
	connect(client, SIGNAL(gameJoinedEventReceived(Event_GameJoined *)), this, SLOT(gameJoined(Event_GameJoined *)));
	connect(client, SIGNAL(maxPingTime(int, int)), this, SLOT(updatePingTime(int, int)));

	tabServer = new TabServer(client);
	connect(tabServer, SIGNAL(chatChannelJoined(const QString &)), this, SLOT(addChatChannelTab(const QString &)));
	myAddTab(tabServer);
	updatePingTime(0, -1);
	
	tabDeckStorage = new TabDeckStorage(client);
	myAddTab(tabDeckStorage);

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
	
	setTabIcon(0, QIcon(pingPixmapGenerator->generatePixmap(15, value, max)));
}

void TabSupervisor::gameJoined(Event_GameJoined *event)
{
	TabGame *tab = new TabGame(client, event->getGameId(), event->getGameDescription(), event->getPlayerId(), event->getSpectator(), event->getSpectatorsCanTalk(), event->getSpectatorsSeeEverything(), event->getResuming());
	connect(tab, SIGNAL(gameClosing(TabGame *)), this, SLOT(gameLeft(TabGame *)));
	myAddTab(tab);
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
	myAddTab(tab);
	chatChannelTabs.insert(channelName, tab);
	setCurrentWidget(tab);
}

void TabSupervisor::chatChannelLeft(TabChatChannel *tab)
{
	emit setMenu(0);

	chatChannelTabs.remove(tab->getChannelName());
	removeTab(indexOf(tab));
}

void TabSupervisor::tabUserEvent()
{
	Tab *tab = static_cast<Tab *>(sender());
	if (tab != currentWidget()) {
		tab->setContentsChanged(true);
		setTabIcon(indexOf(tab), *tabChangedIcon);
	}
	QApplication::alert(this);
}

void TabSupervisor::processChatEvent(ChatEvent *event)
{
	TabChatChannel *tab = chatChannelTabs.value(event->getChannel(), 0);
	if (tab)
		tab->processChatEvent(event);
}

void TabSupervisor::processGameEventContainer(GameEventContainer *cont)
{
	TabGame *tab = gameTabs.value(cont->getGameId());
	if (tab) {
		qDebug() << "gameEvent gameId =" << cont->getGameId();
		tab->processGameEventContainer(cont);
	} else
		qDebug() << "gameEvent: invalid gameId";
}

void TabSupervisor::updateCurrent(int index)
{
	if (index != -1) {
		Tab *tab = static_cast<Tab *>(widget(index));
		if (tab->getContentsChanged()) {
			setTabIcon(index, QIcon());
			tab->setContentsChanged(false);
		}
		emit setMenu(static_cast<Tab *>(widget(index))->getTabMenu());
	} else
		emit setMenu(0);
}
