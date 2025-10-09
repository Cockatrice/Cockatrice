/**
 * @file tab_account.h
 * @ingroup AccountTabs
 * @brief TODO: Document this.
 */

#ifndef TAB_ACCOUNT_H
#define TAB_ACCOUNT_H

#include "tab.h"

#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class AbstractClient;
class Event_AddToList;
class Event_ListRooms;
class Event_RemoveFromList;
class Event_UserJoined;
class Event_UserLeft;
class LineEditUnfocusable;
class Response;
class ServerInfo_User;
class UserInfoBox;
class UserListWidget;

class TabAccount : public Tab
{
    Q_OBJECT
signals:
    void openMessageDialog(const QString &userName, bool focus);
    void userLeft(const QString &userName);
    void userJoined(const ServerInfo_User &userInfo);

private slots:
    void processListUsersResponse(const Response &response);
    void processUserJoinedEvent(const Event_UserJoined &event);
    void processUserLeftEvent(const Event_UserLeft &event);
    void buddyListReceived(const QList<ServerInfo_User> &_buddyList);
    void ignoreListReceived(const QList<ServerInfo_User> &_ignoreList);
    void processAddToListEvent(const Event_AddToList &event);
    void processRemoveFromListEvent(const Event_RemoveFromList &event);
    void addToIgnoreList();
    void addToBuddyList();

private:
    AbstractClient *client;
    UserListWidget *allUsersList;
    UserListWidget *buddyList;
    UserListWidget *ignoreList;
    UserInfoBox *userInfoBox;
    LineEditUnfocusable *addBuddyEdit;
    LineEditUnfocusable *addIgnoreEdit;
    void addToList(const std::string &listName, const QString &userName);

public:
    explicit TabAccount(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User &userInfo);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Account");
    }
};

#endif
