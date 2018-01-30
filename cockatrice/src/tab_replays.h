#ifndef TAB_REPLAYS_H
#define TAB_REPLAYS_H

#include "tab.h"

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
    QToolBar *leftToolBar, *rightToolBar;
    RemoteReplayList_TreeWidget *serverDirView;
    QGroupBox *leftGroupBox, *rightGroupBox;

    QAction *aOpenLocalReplay, *aDeleteLocalReplay, *aOpenRemoteReplay, *aDownload, *aKeep, *aDeleteRemoteReplay;
private slots:
    void actOpenLocalReplay();

    void actDeleteLocalReplay();

    void actOpenRemoteReplay();
    void openRemoteReplayFinished(const Response &r);

    void actDownload();
    void downloadFinished(const Response &r, const CommandContainer &commandContainer, const QVariant &extraData);

    void actKeepRemoteReplay();
    void keepRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer);

    void actDeleteRemoteReplay();
    void deleteRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer);

    void replayAddedEventReceived(const Event_ReplayAdded &event);
signals:
    void openReplay(GameReplay *replay);

public:
    TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi();
    QString getTabText() const
    {
        return tr("Game replays");
    }
};

#endif
