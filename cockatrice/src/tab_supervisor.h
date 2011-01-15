#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include <QTabWidget>
#include <QMap>

class QMenu;
class AbstractClient;
class Tab;
class TabServer;
class TabRoom;
class TabGame;
class TabDeckStorage;
class TabAdmin;
class TabMessage;
class RoomEvent;
class GameEventContainer;
class Event_GameJoined;
class Event_Message;
class ServerInfo_Room;
class ServerInfo_User;

class TabSupervisor : public QTabWidget {
	Q_OBJECT
private:
	QString userName;
	QIcon *tabChangedIcon;
	AbstractClient *client;
	QList<AbstractClient *> localClients;
	TabServer *tabServer;
	TabDeckStorage *tabDeckStorage;
	TabAdmin *tabAdmin;
	QMap<int, TabRoom *> roomTabs;
	QMap<int, TabGame *> gameTabs;
	QMap<QString, TabMessage *> messageTabs;
	void myAddTab(Tab *tab);
public:
	TabSupervisor(QWidget *parent = 0);
	~TabSupervisor();
	void retranslateUi();
	void start(AbstractClient *_client, ServerInfo_User *userInfo);
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
	void addRoomTab(ServerInfo_Room *info, bool setCurrent);
	void roomLeft(TabRoom *tab);
	TabMessage *addMessageTab(const QString &userName, bool focus);
	void processUserLeft(const QString &userName);
	void talkLeft(TabMessage *tab);
	void tabUserEvent();
	void processRoomEvent(RoomEvent *event);
	void processGameEventContainer(GameEventContainer *cont);
	void processMessageEvent(Event_Message *event);
};

#endif
