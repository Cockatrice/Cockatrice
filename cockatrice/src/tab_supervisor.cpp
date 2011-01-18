#include <QApplication>
#include "tab_supervisor.h"
#include "abstractclient.h"
#include "tab_server.h"
#include "tab_room.h"
#include "tab_game.h"
#include "tab_deck_storage.h"
#include "tab_admin.h"
#include "tab_message.h"
#include "protocol_items.h"
#include "pixmapgenerator.h"
#include <QDebug>

TabSupervisor::	TabSupervisor(QWidget *parent)
	: QTabWidget(parent), client(0), tabServer(0), tabDeckStorage(0), tabAdmin(0)
{
	tabChangedIcon = new QIcon(":/resources/icon_tab_changed.svg");
	setElideMode(Qt::ElideRight);
	setIconSize(QSize(15, 15));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(updateCurrent(int)));
}

TabSupervisor::~TabSupervisor()
{
	stop();
	delete tabChangedIcon;
}

void TabSupervisor::retranslateUi()
{
	QList<Tab *> tabs;
	if (tabServer)
		tabs.append(tabServer);
	if (tabDeckStorage)
		tabs.append(tabDeckStorage);
	QMapIterator<int, TabRoom *> roomIterator(roomTabs);
	while (roomIterator.hasNext())
		tabs.append(roomIterator.next().value());
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

void TabSupervisor::start(AbstractClient *_client, ServerInfo_User *userInfo)
{
	client = _client;
	userName = userInfo->getName();
	
	connect(client, SIGNAL(roomEventReceived(RoomEvent *)), this, SLOT(processRoomEvent(RoomEvent *)));
	connect(client, SIGNAL(gameEventContainerReceived(GameEventContainer *)), this, SLOT(processGameEventContainer(GameEventContainer *)));
	connect(client, SIGNAL(gameJoinedEventReceived(Event_GameJoined *)), this, SLOT(gameJoined(Event_GameJoined *)));
	connect(client, SIGNAL(messageEventReceived(Event_Message *)), this, SLOT(processMessageEvent(Event_Message *)));
	connect(client, SIGNAL(maxPingTime(int, int)), this, SLOT(updatePingTime(int, int)));

	tabServer = new TabServer(client, userInfo);
	connect(tabServer, SIGNAL(roomJoined(ServerInfo_Room *, bool)), this, SLOT(addRoomTab(ServerInfo_Room *, bool)));
	connect(tabServer, SIGNAL(openMessageDialog(const QString &, bool)), this, SLOT(addMessageTab(const QString &, bool)));
	connect(tabServer, SIGNAL(userLeft(const QString &)), this, SLOT(processUserLeft(const QString &)));
	myAddTab(tabServer);
	updatePingTime(0, -1);
	
	if (userInfo->getUserLevel() & ServerInfo_User::IsRegistered) {
		tabDeckStorage = new TabDeckStorage(client);
		myAddTab(tabDeckStorage);
	} else
		tabDeckStorage = 0;
	
	if (userInfo->getUserLevel() & ServerInfo_User::IsAdmin) {
		tabAdmin = new TabAdmin(client);
		myAddTab(tabAdmin);
	} else
		tabAdmin = 0;

	retranslateUi();
}

void TabSupervisor::startLocal(const QList<AbstractClient *> &_clients)
{
	localClients = _clients;
	for (int i = 0; i < localClients.size(); ++i)
		connect(localClients[i], SIGNAL(gameEventContainerReceived(GameEventContainer *)), this, SLOT(processGameEventContainer(GameEventContainer *)));
	connect(localClients.first(), SIGNAL(gameJoinedEventReceived(Event_GameJoined *)), this, SLOT(localGameJoined(Event_GameJoined *)));
}

void TabSupervisor::stop()
{
	if ((!client) && localClients.isEmpty())
		return;
	
	if (client) {
		disconnect(client, 0, this, 0);
		client = 0;
	}
	
	if (!localClients.isEmpty()) {
		for (int i = 0; i < localClients.size(); ++i)
			localClients[i]->deleteLater();
		localClients.clear();
		
		emit localGameEnded();
	}

	clear();
	
	tabServer->deleteLater();
	tabServer = 0;
	
	tabDeckStorage->deleteLater();
	tabDeckStorage = 0;
	
	QMapIterator<int, TabRoom *> roomIterator(roomTabs);
	while (roomIterator.hasNext())
		roomIterator.next().value()->deleteLater();
	roomTabs.clear();

	QMapIterator<int, TabGame *> gameIterator(gameTabs);
	while (gameIterator.hasNext())
		gameIterator.next().value()->deleteLater();
	gameTabs.clear();
}

void TabSupervisor::updatePingTime(int value, int max)
{
	if (!tabServer)
		return;
	if (tabServer->getContentsChanged())
		return;
	
	setTabIcon(0, QIcon(PingPixmapGenerator::generatePixmap(15, value, max)));
}

void TabSupervisor::gameJoined(Event_GameJoined *event)
{
	TabGame *tab = new TabGame(QList<AbstractClient *>() << client, event->getGameId(), event->getGameDescription(), event->getPlayerId(), event->getSpectator(), event->getSpectatorsCanTalk(), event->getSpectatorsSeeEverything(), event->getResuming());
	connect(tab, SIGNAL(gameClosing(TabGame *)), this, SLOT(gameLeft(TabGame *)));
	myAddTab(tab);
	gameTabs.insert(event->getGameId(), tab);
	setCurrentWidget(tab);
}

void TabSupervisor::localGameJoined(Event_GameJoined *event)
{
	TabGame *tab = new TabGame(localClients, event->getGameId(), event->getGameDescription(), event->getPlayerId(), event->getSpectator(), event->getSpectatorsCanTalk(), event->getSpectatorsSeeEverything(), event->getResuming());
	connect(tab, SIGNAL(gameClosing(TabGame *)), this, SLOT(gameLeft(TabGame *)));
	myAddTab(tab);
	gameTabs.insert(event->getGameId(), tab);
	setCurrentWidget(tab);
	
	for (int i = 1; i < localClients.size(); ++i) {
		Command_JoinGame *cmd = new Command_JoinGame(0, event->getGameId());
		localClients[i]->sendCommand(cmd);
	}
}

void TabSupervisor::gameLeft(TabGame *tab)
{
	emit setMenu(0);

	gameTabs.remove(tab->getGameId());
	removeTab(indexOf(tab));
	
	if (!localClients.isEmpty())
		stop();
}

void TabSupervisor::addRoomTab(ServerInfo_Room *info, bool setCurrent)
{
	TabRoom *tab = new TabRoom(client, userName, info);
	connect(tab, SIGNAL(roomClosing(TabRoom *)), this, SLOT(roomLeft(TabRoom *)));
	myAddTab(tab);
	roomTabs.insert(info->getRoomId(), tab);
	if (setCurrent)
		setCurrentWidget(tab);
}

void TabSupervisor::roomLeft(TabRoom *tab)
{
	emit setMenu(0);

	roomTabs.remove(tab->getRoomId());
	removeTab(indexOf(tab));
}

TabMessage *TabSupervisor::addMessageTab(const QString &receiverName, bool focus)
{
	if (receiverName == userName)
		return 0;
	
	TabMessage *tab = new TabMessage(client, receiverName);
	connect(tab, SIGNAL(talkClosing(TabMessage *)), this, SLOT(talkLeft(TabMessage *)));
	myAddTab(tab);
	messageTabs.insert(receiverName, tab);
	if (focus)
		setCurrentWidget(tab);
	return tab;
}

void TabSupervisor::talkLeft(TabMessage *tab)
{
	emit setMenu(0);

	messageTabs.remove(tab->getUserName());
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

void TabSupervisor::processRoomEvent(RoomEvent *event)
{
	TabRoom *tab = roomTabs.value(event->getRoomId(), 0);
	if (tab)
		tab->processRoomEvent(event);
}

void TabSupervisor::processGameEventContainer(GameEventContainer *cont)
{
	TabGame *tab = gameTabs.value(cont->getGameId());
	if (tab) {
		qDebug() << "gameEvent gameId =" << cont->getGameId();
		tab->processGameEventContainer(cont, qobject_cast<AbstractClient *>(sender()));
	} else
		qDebug() << "gameEvent: invalid gameId";
}

void TabSupervisor::processMessageEvent(Event_Message *event)
{
	TabMessage *tab = messageTabs.value(event->getSenderName());
	if (!tab)
		tab = messageTabs.value(event->getReceiverName());
	if (!tab)
		tab = addMessageTab(event->getSenderName(), false);
	if (!tab)
		return;
	tab->processMessageEvent(event);
}

void TabSupervisor::processUserLeft(const QString &userName)
{
	TabMessage *tab = messageTabs.value(userName);
	if (tab)
		tab->processUserLeft(userName);
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
