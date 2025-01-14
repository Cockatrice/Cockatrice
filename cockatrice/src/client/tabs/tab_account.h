#ifndef TAB_ACCOUNT_H
#define TAB_ACCOUNT_H

#include "pb/serverinfo_user.pb.h"
#include "tab.h"

class AbstractClient;
class UserList;
class UserInfoBox;
class LineEditUnfocusable;
class UserListManager;
class Event_ListRooms;
class Event_UserJoined;
class Event_UserLeft;
class Response;
class ServerInfo_User;
class Event_AddToList;
class Event_RemoveFromList;

class TabUserLists : public Tab
{
    Q_OBJECT
signals:
    void openMessageDialog(const QString &userName, bool focus);
private slots:
    void addToIgnoreList();
    void addToBuddyList();

private:
    AbstractClient *client;
    UserListManager *userListManager;
    UserInfoBox *userInfoBox;
    LineEditUnfocusable *addBuddyEdit;
    LineEditUnfocusable *addIgnoreEdit;

public:
    TabUserLists(TabSupervisor *_tabSupervisor,
                 AbstractClient *_client,
                 const ServerInfo_User &userInfo,
                 UserListManager *_userListManager);
    void retranslateUi() override;
    QString getTabText() const override
    {
        return tr("Account");
    }
};

#endif
