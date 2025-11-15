/**
 * @file tab_replays.h
 * @ingroup Replays
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_REPLAYS_H
#define TAB_REPLAYS_H

#include "tab.h"

#include <libcockatrice/network/client/abstract/abstract_client.h>

class ServerInfo_User;
class Response;
class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QToolBar;
class QGroupBox;
class RemoteReplayList_TreeWidget;
class GameReplay;
class Event_ReplayAdded;
class CommandContainer;

class TabReplays : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    QTreeView *localDirView;
    QFileSystemModel *localDirModel;
    RemoteReplayList_TreeWidget *serverDirView;
    QGroupBox *leftGroupBox, *rightGroupBox;

    QAction *aOpenLocalReplay, *aRenameLocal, *aNewLocalFolder, *aDeleteLocalReplay;
    QAction *aOpenReplaysFolder;
    QAction *aOpenRemoteReplay, *aDownload, *aKeep, *aDeleteRemoteReplay, *aGetReplayCode;
    QAction *aSubmitReplayCode;

    QGroupBox *createLeftLayout();
    QGroupBox *createRightLayout();

    void setRemoteEnabled(bool enabled);

    void downloadNodeAtIndex(const QModelIndex &curLeft, const QModelIndex &curRight);

private slots:
    void handleConnected(const ServerInfo_User &userInfo);
    void handleConnectionChanged(ClientStatus status);

    void actLocalDoubleClick(const QModelIndex &curLeft);
    void actRenameLocal();
    void actOpenLocalReplay();
    void actNewLocalFolder();
    void actDeleteLocalReplay();

    void actOpenReplaysFolder();

    void actRemoteDoubleClick(const QModelIndex &curLeft);
    void actOpenRemoteReplay();
    void openRemoteReplayFinished(const Response &r);

    void actDownload();
    void downloadFinished(const Response &r, const CommandContainer &commandContainer, const QVariant &extraData);

    void actKeepRemoteReplay();
    void keepRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer);

    void actDeleteRemoteReplay();
    void deleteRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer);

    void actGetReplayCode();
    void getReplayCodeFinished(const Response &r, const CommandContainer &commandContainer);

    void actSubmitReplayCode();
    void submitReplayCodeFinished(const Response &r, const CommandContainer &commandContainer);

    void replayAddedEventReceived(const Event_ReplayAdded &event);
signals:
    void openReplay(GameReplay *replay);

public:
    TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User *currentUserInfo);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Game Replays");
    }
};

#endif
