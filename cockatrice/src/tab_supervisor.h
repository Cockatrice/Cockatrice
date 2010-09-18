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
class TabMessage;
class ChatEvent;
class GameEventContainer;
class Event_GameJoined;
class Event_Message;

class TabSupervisor : public QTabWidget {
	Q_OBJECT
private:
	QIcon *tabChangedIcon;
	AbstractClient *client;
	QList<AbstractClient *> localClients;
	TabServer *tabServer;
	TabDeckStorage *tabDeckStorage;
	QMap<QString, TabChatChannel *> chatChannelTabs;
	QMap<int, TabGame *> gameTabs;
	QMap<QString, TabMessage *> messageTabs;
	void myAddTab(Tab *tab);
public:
	TabSupervisor(QWidget *parent = 0);
	~TabSupervisor();
	void retranslateUi();
	void start(AbstractClient *_client);
	void startLocal(const QList<AbstractClient *> &_clients);
	void stop();
	int getGameCount() const { return gameTabs.size(); }
signals:
	void setMenu(QMenu *menu);
	void localGameEnded();
private slots:
	void updateCurrent(int index);
	void updatePingTime(int value, int max);
	void gameJoined(Event_GameJoined *event);
	void localGameJoined(Event_GameJoined *event);
	void gameLeft(TabGame *tab);
	void addChatChannelTab(const QString &channelName);
	void chatChannelLeft(TabChatChannel *tab);
	TabMessage *addMessageTab(const QString &userName, bool focus);
	void processUserLeft(const QString &userName);
	void talkLeft(TabMessage *tab);
	void tabUserEvent();
	void processChatEvent(ChatEvent *event);
	void processGameEventContainer(GameEventContainer *cont);
	void processMessageEvent(Event_Message *event);
};

#endif
