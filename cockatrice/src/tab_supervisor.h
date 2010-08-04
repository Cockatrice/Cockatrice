#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include <QTabWidget>
#include <QMap>

class QMenu;
class AbstractClient;
class Tab;
class TabServer;
class TabChatChannel;
class TabGame;
class TabDeckStorage;
class ChatEvent;
class GameEventContainer;
class Event_GameJoined;

class TabSupervisor : public QTabWidget {
	Q_OBJECT
private:
	QIcon *tabChangedIcon;
	AbstractClient *client;
	TabServer *tabServer;
	TabDeckStorage *tabDeckStorage;
	QMap<QString, TabChatChannel *> chatChannelTabs;
	QMap<int, TabGame *> gameTabs;
	void myAddTab(Tab *tab);
public:
	TabSupervisor(QWidget *parent = 0);
	~TabSupervisor();
	void retranslateUi();
	void start(AbstractClient *_client);
	void stop();
	int getGameCount() const { return gameTabs.size(); }
signals:
	void setMenu(QMenu *menu);
private slots:
	void updateCurrent(int index);
	void updatePingTime(int value, int max);
	void gameJoined(Event_GameJoined *event);
	void gameLeft(TabGame *tab);
	void addChatChannelTab(const QString &channelName);
	void chatChannelLeft(TabChatChannel *tab);
	void tabUserEvent();
	void processChatEvent(ChatEvent *event);
	void processGameEventContainer(GameEventContainer *cont);
};

#endif
