#include "tab_replays.h"

#include "../../server/pending_command.h"
#include "../../server/remote/remote_replay_list_tree_widget.h"
#include "../../settings/cache_settings.h"
#include "../game_logic/abstract_client.h"
#include "pb/command_replay_delete_match.pb.h"
#include "pb/command_replay_download.pb.h"
#include "pb/command_replay_modify_match.pb.h"
#include "pb/event_replay_added.pb.h"
#include "pb/game_replay.pb.h"
#include "pb/response.pb.h"
#include "pb/response_replay_download.pb.h"
#include "tab_game.h"

#include <QAction>
#include <QApplication>
#include <QFileSystemModel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

TabReplays::TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client) : Tab(_tabSupervisor), client(_client)
{
    localDirModel = new QFileSystemModel(this);
    localDirModel->setRootPath(SettingsCache::instance().getReplaysPath());
    localDirModel->sort(0, Qt::AscendingOrder);

    localDirView = new QTreeView;
    localDirView->setModel(localDirModel);
    localDirView->setColumnHidden(1, true);
    localDirView->setRootIndex(localDirModel->index(localDirModel->rootPath(), 0));
    localDirView->setSortingEnabled(true);
    localDirView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    aNewLocalFolder = new QAction(this);
    aNewLocalFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(aNewLocalFolder, &QAction::triggered, this, &TabReplays::actNewLocalFolder);
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
    leftToolBar->addAction(aNewLocalFolder);
    leftToolBar->addAction(aDeleteLocalReplay);
    rightToolBar->addAction(aOpenRemoteReplay);
    rightToolBar->addAction(aDownload);
    rightToolBar->addAction(aKeep);
    rightToolBar->addAction(aDeleteRemoteReplay);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);

    connect(client, SIGNAL(replayAddedEventReceived(const Event_ReplayAdded &)), this,
            SLOT(replayAddedEventReceived(const Event_ReplayAdded &)));
}

void TabReplays::retranslateUi()
{
    leftGroupBox->setTitle(tr("Local file system"));
    rightGroupBox->setTitle(tr("Server replay storage"));

    aOpenLocalReplay->setText(tr("Watch replay"));
    aNewLocalFolder->setText(tr("New folder"));
    aDeleteLocalReplay->setText(tr("Delete"));
    aOpenRemoteReplay->setText(tr("Watch replay"));
    aDownload->setText(tr("Download replay"));
    aKeep->setText(tr("Toggle expiration lock"));
    aDeleteRemoteReplay->setText(tr("Delete"));
}

void TabReplays::actOpenLocalReplay()
{
    QModelIndexList curLefts = localDirView->selectionModel()->selectedRows();
    for (const auto &curLeft : curLefts) {
        if (localDirModel->isDir(curLeft))
            continue;
        QString filePath = localDirModel->filePath(curLeft);

        QFile f(filePath);
        if (!f.open(QIODevice::ReadOnly))
            continue;
        QByteArray _data = f.readAll();
        f.close();

        GameReplay *replay = new GameReplay;
        replay->ParseFromArray(_data.data(), _data.size());

        emit openReplay(replay);
    }
}

void TabReplays::actNewLocalFolder()
{
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();

    QModelIndex dirIndex;
    if (curLeft.isValid() && !localDirModel->isDir(curLeft)) {
        dirIndex = curLeft.parent();
    } else {
        dirIndex = curLeft;
    }

    bool ok;
    QString folderName =
        QInputDialog::getText(this, tr("New folder"), tr("Name of new folder:"), QLineEdit::Normal, "", &ok);
    if (!ok || folderName.isEmpty())
        return;

    localDirModel->mkdir(dirIndex, folderName);
}

void TabReplays::actDeleteLocalReplay()
{
    QModelIndexList curLefts = localDirView->selectionModel()->selectedRows();

    if (curLefts.isEmpty()) {
        return;
    }

    if (QMessageBox::warning(this, tr("Delete local file"), tr("Are you sure you want to delete the selected files?"),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    for (const auto &curLeft : curLefts) {
        if (curLeft.isValid()) {
            localDirModel->remove(curLeft);
        }
    }
}

void TabReplays::actOpenRemoteReplay()
{
    auto const curRights = serverDirView->getSelectedReplays();

    for (const auto curRight : curRights) {
        if (!curRight) {
            continue;
        }

        Command_ReplayDownload cmd;
        cmd.set_replay_id(curRight->replay_id());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(openRemoteReplayFinished(const Response &)));
        client->sendCommand(pend);
    }
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
    QString dirPath;
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    if (!curLeft.isValid())
        dirPath = localDirModel->rootPath();
    else {
        while (!localDirModel->isDir(curLeft))
            curLeft = curLeft.parent();
        dirPath = localDirModel->filePath(curLeft);
    }

    const auto curRights = serverDirView->getSelectedReplays();

    const auto isNull = [](const auto *replay) { return !replay; };
    if (std::any_of(curRights.begin(), curRights.end(), isNull)) {
        QMessageBox::information(this, tr("Downloading Replays"),
                                 tr("Folder download is not yet supported. Please download replays individually."));
        return;
    }

    for (const auto curRight : curRights) {
        const QString filePath = dirPath + QString("/replay_%1.cor").arg(curRight->replay_id());

        Command_ReplayDownload cmd;
        cmd.set_replay_id(curRight->replay_id());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        pend->setExtraData(filePath);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(downloadFinished(Response, CommandContainer, QVariant)));
        client->sendCommand(pend);
    }
}

void TabReplays::downloadFinished(const Response &r,
                                  const CommandContainer & /* commandContainer */,
                                  const QVariant &extraData)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Response_ReplayDownload &resp = r.GetExtension(Response_ReplayDownload::ext);
    QString filePath = extraData.toString();

    const std::string &_data = resp.replay_data();
    QFile f(filePath);
    f.open(QIODevice::WriteOnly);
    f.write((const char *)_data.data(), _data.size());
    f.close();
}

void TabReplays::actKeepRemoteReplay()
{
    const auto curRights = serverDirView->getSelectedReplayMatches();

    if (curRights.isEmpty()) {
        return;
    }

    for (const auto curRight : curRights) {
        Command_ReplayModifyMatch cmd;
        cmd.set_game_id(curRight->game_id());
        cmd.set_do_not_hide(!curRight->do_not_hide());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(keepRemoteReplayFinished(Response, CommandContainer)));
        client->sendCommand(pend);
    }
}

void TabReplays::keepRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Command_ReplayModifyMatch &cmd =
        commandContainer.session_command(0).GetExtension(Command_ReplayModifyMatch::ext);

    ServerInfo_ReplayMatch temp;
    temp.set_do_not_hide(cmd.do_not_hide());

    serverDirView->updateMatchInfo(cmd.game_id(), temp);
}

void TabReplays::actDeleteRemoteReplay()
{
    const auto curRights = serverDirView->getSelectedReplayMatches();

    if (curRights.isEmpty()) {
        return;
    }

    if (QMessageBox::warning(this, tr("Delete remote replay"),
                             tr("Are you sure you want to delete the selected replays?"),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    for (const auto curRight : curRights) {
        Command_ReplayDeleteMatch cmd;
        cmd.set_game_id(curRight->game_id());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(deleteRemoteReplayFinished(Response, CommandContainer)));
        client->sendCommand(pend);
    }
}

void TabReplays::deleteRemoteReplayFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Command_ReplayDeleteMatch &cmd =
        commandContainer.session_command(0).GetExtension(Command_ReplayDeleteMatch::ext);
    serverDirView->removeMatchInfo(cmd.game_id());
}

void TabReplays::replayAddedEventReceived(const Event_ReplayAdded &event)
{
    serverDirView->addMatchInfo(event.match_info());
}
