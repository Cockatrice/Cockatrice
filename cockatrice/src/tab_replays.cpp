#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QGroupBox>
#include <QHeaderView>
#include <QApplication>
#include <QInputDialog>
#include "tab_replays.h"
#include "remotereplaylist_treewidget.h"
#include "abstractclient.h"
#include "tab_game.h"
#include "settingscache.h"

#include "pending_command.h"
#include "pb/game_replay.pb.h"
#include "pb/response.pb.h"
#include "pb/response_replay_download.pb.h"
#include "pb/command_replay_download.pb.h"

TabReplays::TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client)
	: Tab(_tabSupervisor), client(_client)
{
	localDirModel = new QFileSystemModel(this);
	localDirModel->setRootPath(settingsCache->getReplaysPath());
	
	sortFilter = new QSortFilterProxyModel(this);
	sortFilter->setSourceModel(localDirModel);
	sortFilter->setDynamicSortFilter(true);
	
	localDirView = new QTreeView;
	localDirView->setModel(sortFilter);
	localDirView->setColumnHidden(1, true);
	localDirView->setRootIndex(sortFilter->mapFromSource(localDirModel->index(localDirModel->rootPath(), 0)));
	localDirView->setSortingEnabled(true);
	localDirView->header()->setResizeMode(QHeaderView::ResizeToContents);
	sortFilter->sort(0, Qt::AscendingOrder);
	localDirView->header()->setSortIndicator(0, Qt::AscendingOrder);
	
	leftToolBar = new QToolBar;
	leftToolBar->setOrientation(Qt::Horizontal);
	leftToolBar->setIconSize(QSize(32, 32));
	QHBoxLayout *leftToolBarLayout = new QHBoxLayout;
	leftToolBarLayout->addStretch();
	leftToolBarLayout->addWidget(leftToolBar);
	leftToolBarLayout->addStretch();

	QVBoxLayout *leftVbox = new QVBoxLayout;
	leftVbox->addWidget(localDirView);
	leftVbox->addLayout(leftToolBarLayout);
	leftGroupBox = new QGroupBox;
	leftGroupBox->setLayout(leftVbox);
	
	rightToolBar = new QToolBar;
	rightToolBar->setOrientation(Qt::Horizontal);
	rightToolBar->setIconSize(QSize(32, 32));
	QHBoxLayout *rightToolBarLayout = new QHBoxLayout;
	rightToolBarLayout->addStretch();
	rightToolBarLayout->addWidget(rightToolBar);
	rightToolBarLayout->addStretch();

	serverDirView = new RemoteReplayList_TreeWidget(client);

	QVBoxLayout *rightVbox = new QVBoxLayout;
	rightVbox->addWidget(serverDirView);
	rightVbox->addLayout(rightToolBarLayout);
	rightGroupBox = new QGroupBox;
	rightGroupBox->setLayout(rightVbox);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(leftGroupBox);
	hbox->addWidget(rightGroupBox);
	
	aOpenLocalReplay = new QAction(this);
	aOpenLocalReplay->setIcon(QIcon(":/resources/pencil.svg"));
	connect(aOpenLocalReplay, SIGNAL(triggered()), this, SLOT(actOpenLocalReplay()));
	aOpenRemoteReplay = new QAction(this);
	aOpenRemoteReplay->setIcon(QIcon(":/resources/pencil.svg"));
	connect(aOpenRemoteReplay, SIGNAL(triggered()), this, SLOT(actOpenRemoteReplay()));
	aDownload = new QAction(this);
	aDownload->setIcon(QIcon(":/resources/arrow_left_green.svg"));
	connect(aDownload, SIGNAL(triggered()), this, SLOT(actDownload()));
	
	leftToolBar->addAction(aOpenLocalReplay);
	rightToolBar->addAction(aOpenRemoteReplay);
	rightToolBar->addAction(aDownload);
	
	retranslateUi();
	setLayout(hbox);
}

void TabReplays::retranslateUi()
{
	leftGroupBox->setTitle(tr("Local file system"));
	rightGroupBox->setTitle(tr("Server replay storage"));
	
	aOpenLocalReplay->setText(tr("Watch replay"));
	aOpenRemoteReplay->setText(tr("Watch replay"));
	aDownload->setText(tr("Download replay"));
}

void TabReplays::actOpenLocalReplay()
{
	QModelIndex curLeft = sortFilter->mapToSource(localDirView->selectionModel()->currentIndex());
	if (localDirModel->isDir(curLeft))
		return;
	QString filePath = localDirModel->filePath(curLeft);
	
	QFile f(filePath);
	if (!f.open(QIODevice::ReadOnly))
		return;
	QByteArray data = f.readAll();
	f.close();
	
	GameReplay *replay = new GameReplay;
	replay->ParseFromArray(data.data(), data.size());
	
	emit openReplay(replay);
}

void TabReplays::actOpenRemoteReplay()
{
	ServerInfo_Replay const *curRight = serverDirView->getCurrentItem();
	if (!curRight)
		return;
	
	Command_ReplayDownload cmd;
	cmd.set_game_id(curRight->game_id());
	
	PendingCommand *pend = client->prepareSessionCommand(cmd);
	connect(pend, SIGNAL(finished(const Response &)), this, SLOT(openRemoteReplayFinished(const Response &)));
	client->sendCommand(pend);
}

void TabReplays::openRemoteReplayFinished(const Response &r)
{
	const Response_ReplayDownload &resp = r.GetExtension(Response_ReplayDownload::ext);
	GameReplay *replay = new GameReplay;
	replay->ParseFromString(resp.replay_data());
	
	emit openReplay(replay);
}

void TabReplays::actDownload()
{
	QString filePath;
	QModelIndex curLeft = sortFilter->mapToSource(localDirView->selectionModel()->currentIndex());
	if (!curLeft.isValid())
		filePath = localDirModel->rootPath();
	else {
		while (!localDirModel->isDir(curLeft))
			curLeft = curLeft.parent();
		filePath = localDirModel->filePath(curLeft);
	}

	ServerInfo_Replay const *curRight = serverDirView->getCurrentItem();
	if (!curRight)
		return;
	filePath += QString("/game_%1.cor").arg(curRight->game_id());
	
	Command_ReplayDownload cmd;
	cmd.set_game_id(curRight->game_id());
	
	PendingCommand *pend = client->prepareSessionCommand(cmd);
	pend->setExtraData(filePath);
	connect(pend, SIGNAL(finished(const Response &)), this, SLOT(downloadFinished(const Response &)));
	client->sendCommand(pend);
}

void TabReplays::downloadFinished(const Response &r)
{
	const Response_ReplayDownload &resp = r.GetExtension(Response_ReplayDownload::ext);
	
	PendingCommand *pend = static_cast<PendingCommand *>(sender());
	QString filePath = pend->getExtraData().toString();
	
	const std::string &data = resp.replay_data();
	QFile f(filePath);
	f.open(QIODevice::WriteOnly);
	f.write((const char *) data.data(), data.size());
	f.close();
}
