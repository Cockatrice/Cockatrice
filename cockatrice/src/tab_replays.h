#ifndef TAB_REPLAYS_H
#define TAB_REPLAYS_H

#include "tab.h"

class Response;
class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;
class QToolBar;
class QGroupBox;
class RemoteReplayList_TreeWidget;
class GameReplay;
class Event_ReplayAdded;

class TabReplays : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	QTreeView *localDirView;
	QFileSystemModel *localDirModel;
	QSortFilterProxyModel *sortFilter;
	QToolBar *leftToolBar, *rightToolBar;
	RemoteReplayList_TreeWidget *serverDirView;
	QGroupBox *leftGroupBox, *rightGroupBox;
	
	QAction *aOpenLocalReplay, *aOpenRemoteReplay, *aDownload, *aKeep;
private slots:
	void actOpenLocalReplay();
	
	void actOpenRemoteReplay();
	void openRemoteReplayFinished(const Response &r);
	
	void actDownload();
	void downloadFinished(const Response &r);
	
	void actKeepRemoteReplay();
	void keepRemoteReplayFinished(const Response &r);
	
	void replayAddedEventReceived(const Event_ReplayAdded &event);
signals:
	void openReplay(GameReplay *replay);
public:
	TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client);
	void retranslateUi();
	QString getTabText() const { return tr("Game replays"); }
};

#endif
