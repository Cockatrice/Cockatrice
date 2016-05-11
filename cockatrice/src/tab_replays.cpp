#include <QTreeView>
#include <QFileSystemModel>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QGroupBox>
#include <QHeaderView>
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
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
#include "pb/command_replay_modify_match.pb.h"
#include "pb/command_replay_delete_match.pb.h"
#include "pb/event_replay_added.pb.h"

TabReplays::TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client)
    : Tab(_tabSupervisor), client(_client)
{
    localDirModel = new QFileSystemModel(this);
    localDirModel->setRootPath(settingsCache->getReplaysPath());
    localDirModel->sort(0, Qt::AscendingOrder);
    
    localDirView = new QTreeView;
    localDirView->setModel(localDirModel);
    localDirView->setColumnHidden(1, true);
    localDirView->setRootIndex(localDirModel->index(localDirModel->rootPath(), 0));
    localDirView->setSortingEnabled(true);
    localDirView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
    aOpenLocalReplay->setIcon(QPixmap("theme:icons/view"));
    connect(aOpenLocalReplay, SIGNAL(triggered()), this, SLOT(actOpenLocalReplay()));
    connect(localDirView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actOpenLocalReplay()));
    aDeleteLocalReplay = new QAction(this);
    aDeleteLocalReplay->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteLocalReplay, SIGNAL(triggered()), this, SLOT(actDeleteLocalReplay()));
    aOpenRemoteReplay = new QAction(this);
    aOpenRemoteReplay->setIcon(QPixmap("theme:icons/view"));
    connect(aOpenRemoteReplay, SIGNAL(triggered()), this, SLOT(actOpenRemoteReplay()));
    connect(serverDirView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actOpenRemoteReplay()));
    aDownload = new QAction(this);
    aDownload->setIcon(QPixmap("theme:icons/arrow_left_green"));
    connect(aDownload, SIGNAL(triggered()), this, SLOT(actDownload()));
    aKeep = new QAction(this);
    aKeep->setIcon(QPixmap("theme:icons/lock"));
    connect(aKeep, SIGNAL(triggered()), this, SLOT(actKeepRemoteReplay()));
    aDeleteRemoteReplay = new QAction(this);
    aDeleteRemoteReplay->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteRemoteReplay, SIGNAL(triggered()), this, SLOT(actDeleteRemoteReplay()));
    
    leftToolBar->addAction(aOpenLocalReplay);
    leftToolBar->addAction(aDeleteLocalReplay);
    rightToolBar->addAction(aOpenRemoteReplay);
    rightToolBar->addAction(aDownload);
    rightToolBar->addAction(aKeep);
    rightToolBar->addAction(aDeleteRemoteReplay);
    
    retranslateUi();

    QWidget * mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);
    
    connect(client, SIGNAL(replayAddedEventReceived(const Event_ReplayAdded &)), this, SLOT(replayAddedEventReceived(const Event_ReplayAdded &)));
}

void TabReplays::retranslateUi()
{
    leftGroupBox->setTitle(tr("Local file system"));
    rightGroupBox->setTitle(tr("Server replay storage"));
    
    aOpenLocalReplay->setText(tr("Watch replay"));
    aDeleteLocalReplay->setText(tr("Delete"));
    aOpenRemoteReplay->setText(tr("Watch replay"));
    aDownload->setText(tr("Download replay"));
    aKeep->setText(tr("Toggle expiration lock"));
    aDeleteRemoteReplay->setText(tr("Delete"));
}

void TabReplays::actOpenLocalReplay()
{
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
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

void TabReplays::actDeleteLocalReplay()
{
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    if (QMessageBox::warning(this, tr("Delete local file"), tr("Are you sure you want to delete \"%1\"?").arg(localDirModel->fileName(curLeft)), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    
    localDirModel->remove(curLeft);
}

void TabReplays::actOpenRemoteReplay()
{
    ServerInfo_Replay const *curRight = serverDirView->getCurrentReplay();
    if (!curRight)
        return;
    
    Command_ReplayDownload cmd;
    cmd.set_replay_id(curRight->replay_id());
    
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(openRemoteReplayFinished(const Response &)));
    client->sendCommand(pend);
}

void TabReplays::openRemoteReplayFinished(const Response &r)
{
    if (r.response_code() != Response::RespOk)
        return;
    
    const Response_ReplayDownload &resp = r.GetExtension(Response_ReplayDownload::ext);
    GameReplay *replay = new GameReplay;
    replay->ParseFromString(resp.replay_data());
    
    emit openReplay(replay);
}

void TabReplays::actDownload()
{
    QString filePath;
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    if (!curLeft.isValid())
        filePath = localDirModel->rootPath();
    else {
        while (!localDirModel->isDir(curLeft))
            curLeft = curLeft.parent();
        filePath = localDirModel->filePath(curLeft);
    }

    ServerInfo_Replay const *curRight = serverDirView->getCurrentReplay();
    if (!curRight)
        return;
    filePath += QString("/replay_%1.cor").arg(curRight->replay_id());
    
    Command_ReplayDownload cmd;
    cmd.set_replay_id(curRight->replay_id());
    
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    pend->setExtraData(filePath);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(downloadFinished(Response, CommandContainer, QVariant)));
    client->sendCommand(pend);
}

void TabReplays::downloadFinished(const Response &r, const CommandContainer & /* commandContainer */, const QVariant &extraData)
{
    if (r.response_code() != Response::RespOk)
        return;
    
    const Response_ReplayDownload &resp = r.GetExtension(Response_ReplayDownload::ext);
    QString filePath = extraData.toString();
    
    const std::string &data = resp.replay_data();
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.write((const char *) data.data(), data.size());
    f.close();
}

void TabReplays::actKeepRemoteReplay()
{
    ServerInfo_ReplayMatch const *curRight = serverDirView->getCurrentReplayMatch();
    if (!curRight)
        return;
    
    Command_ReplayModifyMatch cmd;
    cmd.set_game_id(curRight->game_id());
    cmd.set_do_not_hide(!curRight->do_not_hide());
    
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(keepRemoteReplayFinished(Response, CommandContainer)));
    client->sendCommand(pend);
}

void TabReplays::keepRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk)
        return;
    
    const Command_ReplayModifyMatch &cmd = commandContainer.session_command(0).GetExtension(Command_ReplayModifyMatch::ext);
    
    ServerInfo_ReplayMatch temp;
    temp.set_do_not_hide(cmd.do_not_hide());
                    
    serverDirView->updateMatchInfo(cmd.game_id(), temp);
}

void TabReplays::actDeleteRemoteReplay()
{
    ServerInfo_ReplayMatch const *curRight = serverDirView->getCurrentReplayMatch();
    if (!curRight)
        return;
    if (QMessageBox::warning(this, tr("Delete remote replay"), tr("Are you sure you want to delete the replay of game %1?").arg(curRight->game_id()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    
    Command_ReplayDeleteMatch cmd;
    cmd.set_game_id(curRight->game_id());
    
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(deleteRemoteReplayFinished(Response, CommandContainer)));
    client->sendCommand(pend);
}

void TabReplays::deleteRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk)
        return;
    
    const Command_ReplayDeleteMatch &cmd = commandContainer.session_command(0).GetExtension(Command_ReplayDeleteMatch::ext);
    serverDirView->removeMatchInfo(cmd.game_id());
}

void TabReplays::replayAddedEventReceived(const Event_ReplayAdded &event)
{
    serverDirView->addMatchInfo(event.match_info());
}
