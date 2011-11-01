#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include <QTabWidget>
#include <QMap>
#include <QAbstractButton>

class QMenu;
class AbstractClient;
class Tab;
class TabServer;
class TabRoom;
class TabGame;
class TabDeckStorage;
class TabAdmin;
class TabMessage;
class TabUserLists;
class RoomEvent;
class GameEventContainer;
class Event_GameJoined;
class Event_Message;
class ServerInfo_Room;
class ServerInfo_User;

class CloseButton : public QAbstractButton {
	Q_OBJECT
public:
	CloseButton(QWidget *parent = 0);
	QSize sizeHint() const;
	inline QSize minimumSizeHint() const { return sizeHint(); }
protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void paintEvent(QPaintEvent *event);
};

class TabSupervisor : public QTabWidget {
	Q_OBJECT
private:
	ServerInfo_User *userInfo;
	QIcon *tabChangedIcon;
	AbstractClient *client;
	QList<AbstractClient *> localClients;
	TabServer *tabServer;
	TabUserLists *tabUserLists;
	TabDeckStorage *tabDeckStorage;
	TabAdmin *tabAdmin;
	QMap<int, TabRoom *> roomTabs;
	QMap<int, TabGame *> gameTabs;
	QMap<QString, TabMessage *> messageTabs;
	int myAddTab(Tab *tab);
	void addCloseButtonToTab(Tab *tab, int tabIndex);
public:
	TabSupervisor(QWidget *parent = 0);
	~TabSupervisor();
	void retranslateUi();
	void start(AbstractClient *_client, ServerInfo_User *userInfo);
	void startLocal(const QList<AbstractClient *> &_clients);
	void stop();
	int getGameCount() const { return gameTabs.size(); }
	TabUserLists *getUserListsTab() const { return tabUserLists; }
	ServerInfo_User *getUserInfo() const { return userInfo; }
	bool getAdminLocked() const;
	int getUserLevel() const;
signals:
	void setMenu(QMenu *menu);
	void localGameEnded();
private slots:
	void closeButtonPressed();
	void updateCurrent(int index);
	void updatePingTime(int value, int max);
	void gameJoined(Event_GameJoined *event);
	void localGameJoined(Event_GameJoined *event);
	void gameLeft(TabGame *tab);
	void addRoomTab(ServerInfo_Room *info, bool setCurrent);
	void roomLeft(TabRoom *tab);
	TabMessage *addMessageTab(const QString &userName, bool focus);
	void processUserLeft(const QString &userName);
	void processUserJoined(const QString &userName);
	void talkLeft(TabMessage *tab);
	void tabUserEvent(bool globalEvent);
	void processRoomEvent(RoomEvent *event);
	void processGameEventContainer(GameEventContainer *cont);
	void processMessageEvent(Event_Message *event);
};

#endif
