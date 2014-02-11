#ifndef USER_CONTEXT_MENU_H
#define USER_CONTEXT_MENU_H

#include <QObject>
#include "user_level.h"

class QAction;
class TabSupervisor;
class TabGame;
class QPoint;
class CommandContainer;
class Response;
class AbstractClient;

class UserContextMenu : public QObject {
    Q_OBJECT
private:
    AbstractClient *client;
    const TabSupervisor *tabSupervisor;
    TabGame *game;
    
    QAction *aUserName;
    QAction *aDetails;
    QAction *aShowGames;
    QAction *aChat;
    QAction *aAddToBuddyList, *aRemoveFromBuddyList;
    QAction *aAddToIgnoreList, *aRemoveFromIgnoreList;
    QAction *aKick;
    QAction *aBan;
signals:
    void openMessageDialog(const QString &userName, bool focus);
private slots:
    void banUser_processUserInfoResponse(const Response &resp);
    void banUser_dialogFinished();
    void gamesOfUserReceived(const Response &resp, const CommandContainer &commandContainer);
public:
    UserContextMenu(const TabSupervisor *_tabSupervisor, QWidget *_parent, TabGame *_game = 0);
    void retranslateUi();
    void showContextMenu(const QPoint &pos, const QString &userName, UserLevelFlags userLevel, int playerId = -1);
};

#endif
