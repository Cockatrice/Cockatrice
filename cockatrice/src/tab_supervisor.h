#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include <QTabWidget>
#include <QMap>

class Client;
class TabServer;
class TabChatChannel;
class TabGame;
class ChatEvent;
class GameEvent;
class Event_GameJoined;

class TabSupervisor : public QTabWidget {
	Q_OBJECT
private:
	Client *client;
	TabServer *tabServer;
	QMap<QString, TabChatChannel *> chatChannelTabs;
	QMap<int, TabGame *> gameTabs;
public:
	TabSupervisor(QWidget *parent = 0);
	void retranslateUi();
	void start(Client *_client);
	void stop();
private slots:
	void gameJoined(Event_GameJoined *event);
	void addChatChannelTab(const QString &channelName);
	void processChatEvent(ChatEvent *event);
	void processGameEvent(GameEvent *event);
};

#endif