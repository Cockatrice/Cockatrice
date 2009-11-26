#include "tab_supervisor.h"
#include "client.h"
#include "tab_server.h"
#include "tab_chatchannel.h"
#include "tab_game.h"
#include "tab_deck_storage.h"
#include "protocol_items.h"
#include <QPainter>

TabSupervisor::	TabSupervisor(QWidget *parent)
	: QTabWidget(parent), client(0), tabServer(0), tabDeckStorage(0)
{
	setIconSize(QSize(15, 15));
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
	QPixmap pixmap(15, 15);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	QColor color;
	if (max == -1)
		color = Qt::black;
	else
		color.setHsv(120 * (1.0 - ((double) value / max)), 255, 255);
	
	QRadialGradient g(QPointF((double) pixmap.width() / 2, (double) pixmap.height() / 2), qMin(pixmap.width(), pixmap.height()) / 2.0);
	g.setColorAt(0, color);
	g.setColorAt(1, Qt::transparent);
	painter.fillRect(0, 0, pixmap.width(), pixmap.height(), QBrush(g));
	
	setTabIcon(0, QIcon(pixmap));
}

void TabSupervisor::gameJoined(Event_GameJoined *event)
{
	TabGame *tab = new TabGame(client, event->getGameId(), event->getPlayerId(), event->getSpectator());
	addTab(tab, tr("Game %1").arg(event->getGameId()));
	gameTabs.insert(event->getGameId(), tab);
	setCurrentWidget(tab);
}

void TabSupervisor::addChatChannelTab(const QString &channelName)
{
	TabChatChannel *tab = new TabChatChannel(client, channelName);
	addTab(tab, channelName);
	chatChannelTabs.insert(channelName, tab);
	setCurrentWidget(tab);
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
