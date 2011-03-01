#ifndef TAB_USERLISTS_H
#define TAB_USERLISTS_H

#include "tab.h"

class AbstractClient;
class UserList;
class UserInfoBox;

class Event_ListRooms;
class Event_UserJoined;
class Event_UserLeft;
class ProtocolResponse;
class ServerInfo_User;
class Event_AddToList;
class Event_RemoveFromList;

class TabUserLists : public Tab {
	Q_OBJECT
signals:
	void openMessageDialog(const QString &userName, bool focus);
	void userLeft(const QString &userName);
	void userJoined(const QString &userName);
private slots:
	void processListUsersResponse(ProtocolResponse *response);
	void processUserJoinedEvent(Event_UserJoined *event);
	void processUserLeftEvent(Event_UserLeft *event);
	void buddyListReceived(const QList<ServerInfo_User *> &_buddyList);
	void ignoreListReceived(const QList<ServerInfo_User *> &_ignoreList);
	void processAddToListEvent(Event_AddToList *event);
	void processRemoveFromListEvent(Event_RemoveFromList *event);
private:
	AbstractClient *client;
	UserList *allUsersList;
	UserList *buddyList;
	UserList *ignoreList;
	UserInfoBox *userInfoBox;
public:
	TabUserLists(TabSupervisor *_tabSupervisor, AbstractClient *_client, ServerInfo_User *userInfo, QWidget *parent = 0);
	void retranslateUi();
	QString getTabText() const { return tr("User lists"); }
	UserList *getBuddyList() const { return buddyList; }
	UserList *getIgnoreList() const { return ignoreList; }
};

#endif
