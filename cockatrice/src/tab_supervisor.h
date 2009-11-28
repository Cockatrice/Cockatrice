#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include <QTabWidget>
#include <QMap>

class QMenu;
class Client;
class TabServer;
class TabChatChannel;
class TabGame;
class TabDeckStorage;
class ChatEvent;
class GameEvent;
class Event_GameJoined;

class TabSupervisor : public QTabWidget {
	Q_OBJECT
private:
	Client *client;
	TabServer *tabServer;
	TabDeckStorage *tabDeckStorage;
	QMap<QString, TabChatChannel *> chatChannelTabs;
	QMap<int, TabGame *> gameTabs;
public:
	TabSupervisor(QWidget *parent = 0);
	void retranslateUi();
	void start(Client *_client);
	void stop();
signals:
	void setMenu(QMenu *menu);
private slots:
	void updateMenu(int index);
	void updatePingTime(int value, int max);
	void gameJoined(Event_GameJoined *event);
	void addChatChannelTab(const QString &channelName);
	void processChatEvent(ChatEvent *event);
	void processGameEvent(GameEvent *event);
};

#endif