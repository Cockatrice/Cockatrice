/**
 * @file user_context_menu.h
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef USER_CONTEXT_MENU_H
#define USER_CONTEXT_MENU_H

#include <QObject>
#include <libcockatrice/network/server/remote/user_level.h>

class AbstractGame;
class UserListProxy;
class AbstractClient;
class ChatView;
class CommandContainer;
class QAction;
class QMenu;
class QPoint;
class Response;
class ServerInfo_User;
class TabSupervisor;

class UserContextMenu : public QObject
{
    Q_OBJECT
private:
    AbstractClient *client;
    TabSupervisor *tabSupervisor;
    const UserListProxy *userListProxy;
    AbstractGame *game;

    QAction *aUserName;
    QAction *aDetails;
    QAction *aShowGames;
    QAction *aChat;
    QAction *aAddToBuddyList, *aRemoveFromBuddyList;
    QAction *aAddToIgnoreList, *aRemoveFromIgnoreList;
    QAction *aKick;
    QAction *aBan, *aBanHistory;
    QAction *aPromoteToMod, *aDemoteFromMod;
    QAction *aPromoteToJudge, *aDemoteFromJudge;
    QAction *aWarnUser, *aWarnHistory;
    QAction *aGetAdminNotes;
signals:
    void openMessageDialog(const QString &userName, bool focus);
private slots:
    void banUser_processUserInfoResponse(const Response &resp);
    void warnUser_processGetWarningsListResponse(const Response &r);
    void warnUser_processUserInfoResponse(const Response &resp);
    void banUserHistory_processResponse(const Response &resp);
    void warnUserHistory_processResponse(const Response &resp);
    void getAdminNotes_processResponse(const Response &resp);
    void adjustMod_processUserResponse(const Response &resp, const CommandContainer &commandContainer);
    void banUser_dialogFinished();
    void warnUser_dialogFinished();
    void updateAdminNotes_dialogFinished();
    void gamesOfUserReceived(const Response &resp, const CommandContainer &commandContainer);

public:
    UserContextMenu(TabSupervisor *_tabSupervisor, QWidget *_parent, AbstractGame *_game = 0);
    void retranslateUi();
    void showContextMenu(const QPoint &pos,
                         const QString &userName,
                         UserLevelFlags userLevel,
                         bool online = true,
                         int playerId = -1);
    void showContextMenu(const QPoint &pos, const QString &userName, UserLevelFlags userLevel, ChatView *chatView);
    void showContextMenu(const QPoint &pos,
                         const QString &userName,
                         UserLevelFlags userLevel,
                         bool online,
                         int playerId,
                         const QString &deckHash,
                         ChatView *chatView = nullptr);

    const UserListProxy *getUserListProxy() const
    {
        return userListProxy;
    }

    // Individual action entry points — used by UserInfoPopup to trigger
    // actions without re-running the full context menu flow.
    void execChat(const QString &userName);
    void execDetails(const QString &userName);
    void execShowGames(const QString &userName);
    void execAddToBuddy(const QString &userName);
    void execRemoveFromBuddy(const QString &userName);
    void execAddToIgnore(const QString &userName);
    void execRemoveFromIgnore(const QString &userName);
    void execKick(int playerId);
    void execBan(const QString &userName);
    void execWarn(const QString &userName);
    void execBanHistory(const QString &userName);
    void execWarnHistory(const QString &userName);
    void execAdminNotes(const QString &userName);
    void execAdjustMod(const QString &userName, bool shouldBeMod);
    void execAdjustJudge(const QString &userName, bool shouldBeJudge);
};

#endif
