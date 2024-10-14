#ifndef USER_CONTEXT_MENU_H
#define USER_CONTEXT_MENU_H

#include "user_level.h"

#include <QObject>

class AbstractClient;
class ChatView;
class CommandContainer;
class QAction;
class QMenu;
class QPoint;
class Response;
class ServerInfo_User;
class TabGame;
class TabSupervisor;

class UserContextMenu : public QObject
{
    Q_OBJECT
private:
    AbstractClient *client;
    TabSupervisor *tabSupervisor;
    TabGame *game;

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
signals:
    void openMessageDialog(const QString &userName, bool focus);
private slots:
    void banUser_processUserInfoResponse(const Response &resp);
    void warnUser_processGetWarningsListResponse(const Response &r);
    void warnUser_processUserInfoResponse(const Response &resp);
    void banUserHistory_processResponse(const Response &resp);
    void warnUserHistory_processResponse(const Response &resp);
    void adjustMod_processUserResponse(const Response &resp, const CommandContainer &commandContainer);
    void banUser_dialogFinished();
    void warnUser_dialogFinished();
    void gamesOfUserReceived(const Response &resp, const CommandContainer &commandContainer);

public:
    UserContextMenu(TabSupervisor *_tabSupervisor, QWidget *_parent, TabGame *_game = 0);
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
};

#endif
