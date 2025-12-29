#include "tab_replays.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/widgets/server/remote/remote_replay_list_tree_widget.h"
#include "tab_game.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileSystemModel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QUrl>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_replay_delete_match.pb.h>
#include <libcockatrice/protocol/pb/command_replay_download.pb.h>
#include <libcockatrice/protocol/pb/command_replay_get_code.pb.h>
#include <libcockatrice/protocol/pb/command_replay_modify_match.pb.h>
#include <libcockatrice/protocol/pb/command_replay_submit_code.pb.h>
#include <libcockatrice/protocol/pb/event_replay_added.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_replay_download.pb.h>
#include <libcockatrice/protocol/pb/response_replay_get_code.pb.h>
#include <libcockatrice/protocol/pending_command.h>

TabReplays::TabReplays(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User *currentUserInfo)
    : Tab(_tabSupervisor), client(_client)
{
    leftGroupBox = createLeftLayout();
    rightGroupBox = createRightLayout();

    // combine layouts
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(leftGroupBox);
    hbox->addWidget(rightGroupBox);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);

    connect(client, &AbstractClient::replayAddedEventReceived, this, &TabReplays::replayAddedEventReceived);

    connect(client, &AbstractClient::userInfoChanged, this, &TabReplays::handleConnected);
    connect(client, &AbstractClient::statusChanged, this, &TabReplays::handleConnectionChanged);
    setRemoteEnabled(currentUserInfo && currentUserInfo->user_level() & ServerInfo_User::IsRegistered);
}

QGroupBox *TabReplays::createLeftLayout()
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

    // Left side layout
    /* put an invisible dummy QToolBar in the leftmost column so that the main toolbar is centered.
     * Really ugly workaround, but I couldn't figure out the proper way to make it centered */
    QToolBar *dummyToolBar = new QToolBar(this);
    QSizePolicy sizePolicy = dummyToolBar->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    dummyToolBar->setSizePolicy(sizePolicy);
    dummyToolBar->setVisible(false);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setOrientation(Qt::Horizontal);
    toolBar->setIconSize(QSize(32, 32));

    QToolBar *rightmostToolBar = new QToolBar(this);
    rightmostToolBar->setOrientation(Qt::Horizontal);
    rightmostToolBar->setIconSize(QSize(32, 32));

    QGridLayout *toolBarLayout = new QGridLayout;
    toolBarLayout->addWidget(dummyToolBar, 0, 0, Qt::AlignLeft);
    toolBarLayout->addWidget(toolBar, 0, 1, Qt::AlignHCenter);
    toolBarLayout->addWidget(rightmostToolBar, 0, 2, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(localDirView);
    vbox->addLayout(toolBarLayout);
    QGroupBox *groupBox = new QGroupBox;
    groupBox->setLayout(vbox);

    // Left side actions
    aOpenLocalReplay = new QAction(this);
    aOpenLocalReplay->setIcon(QPixmap("theme:icons/view"));
    connect(aOpenLocalReplay, &QAction::triggered, this, &TabReplays::actOpenLocalReplay);
    connect(localDirView, &QTreeView::doubleClicked, this, &TabReplays::actOpenLocalReplay);
    aRenameLocal = new QAction(this);
    aRenameLocal->setIcon(QPixmap("theme:icons/rename"));
    connect(aRenameLocal, &QAction::triggered, this, &TabReplays::actRenameLocal);
    aNewLocalFolder = new QAction(this);
    aNewLocalFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(aNewLocalFolder, &QAction::triggered, this, &TabReplays::actNewLocalFolder);
    aDeleteLocalReplay = new QAction(this);
    aDeleteLocalReplay->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteLocalReplay, &QAction::triggered, this, &TabReplays::actDeleteLocalReplay);

    aOpenReplaysFolder = new QAction(this);
    aOpenReplaysFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(aOpenReplaysFolder, &QAction::triggered, this, &TabReplays::actOpenReplaysFolder);

    // Add actions to toolbars
    toolBar->addAction(aOpenLocalReplay);
    toolBar->addAction(aRenameLocal);
    toolBar->addAction(aNewLocalFolder);
    toolBar->addAction(aDeleteLocalReplay);

    rightmostToolBar->addAction(aOpenReplaysFolder);

    return groupBox;
}

QGroupBox *TabReplays::createRightLayout()
{
    serverDirView = new RemoteReplayList_TreeWidget(client);

    // Right side layout
    /* put an invisible dummy QToolBar in the leftmost column so that the main toolbar is centered.
     * Really ugly workaround, but I couldn't figure out the proper way to make it centered */
    QToolBar *dummyToolBar = new QToolBar(this);
    QSizePolicy sizePolicy = dummyToolBar->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    dummyToolBar->setSizePolicy(sizePolicy);
    dummyToolBar->setVisible(false);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setOrientation(Qt::Horizontal);
    toolBar->setIconSize(QSize(32, 32));

    QToolBar *rightmostToolBar = new QToolBar(this);
    rightmostToolBar->setOrientation(Qt::Horizontal);
    rightmostToolBar->setIconSize(QSize(32, 32));

    QGridLayout *toolBarLayout = new QGridLayout;
    toolBarLayout->addWidget(dummyToolBar, 0, 0, Qt::AlignLeft);
    toolBarLayout->addWidget(toolBar, 0, 1, Qt::AlignHCenter);
    toolBarLayout->addWidget(rightmostToolBar, 0, 2, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(serverDirView);
    vbox->addLayout(toolBarLayout);
    QGroupBox *groupBox = new QGroupBox;
    groupBox->setLayout(vbox);

    // Right side actions
    aOpenRemoteReplay = new QAction(this);
    aOpenRemoteReplay->setIcon(QPixmap("theme:icons/view"));
    connect(aOpenRemoteReplay, &QAction::triggered, this, &TabReplays::actOpenRemoteReplay);
    connect(serverDirView, &QTreeView::doubleClicked, this, &TabReplays::actOpenRemoteReplay);
    aDownload = new QAction(this);
    aDownload->setIcon(QPixmap("theme:icons/arrow_left_green"));
    connect(aDownload, &QAction::triggered, this, &TabReplays::actDownload);
    aKeep = new QAction(this);
    aKeep->setIcon(QPixmap("theme:icons/lock"));
    connect(aKeep, &QAction::triggered, this, &TabReplays::actKeepRemoteReplay);
    aDeleteRemoteReplay = new QAction(this);
    aDeleteRemoteReplay->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteRemoteReplay, &QAction::triggered, this, &TabReplays::actDeleteRemoteReplay);
    aGetReplayCode = new QAction(this);
    aGetReplayCode->setIcon(QPixmap("theme:icons/share"));
    connect(aGetReplayCode, &QAction::triggered, this, &TabReplays::actGetReplayCode);

    aSubmitReplayCode = new QAction(this);
    aSubmitReplayCode->setIcon(QPixmap("theme:icons/search"));
    connect(aSubmitReplayCode, &QAction::triggered, this, &TabReplays::actSubmitReplayCode);

    // Add actions to toolbars
    toolBar->addAction(aOpenRemoteReplay);
    toolBar->addAction(aDownload);
    toolBar->addAction(aKeep);
    toolBar->addAction(aDeleteRemoteReplay);
    toolBar->addAction(aGetReplayCode);

    rightmostToolBar->addAction(aSubmitReplayCode);

    return groupBox;
}

void TabReplays::retranslateUi()
{
    leftGroupBox->setTitle(tr("Local file system"));
    rightGroupBox->setTitle(tr("Server replay storage"));

    aOpenLocalReplay->setText(tr("Watch replay"));
    aRenameLocal->setText(tr("Rename"));
    aNewLocalFolder->setText(tr("New folder"));
    aDeleteLocalReplay->setText(tr("Delete"));
    aOpenReplaysFolder->setText(tr("Open replays folder"));
    aOpenRemoteReplay->setText(tr("Watch replay"));
    aDownload->setText(tr("Download replay"));
    aKeep->setText(tr("Toggle expiration lock"));
    aDeleteRemoteReplay->setText(tr("Delete"));
    aGetReplayCode->setText(tr("Get replay share code"));

    aSubmitReplayCode->setText(tr("Look up replay by share code"));
}

void TabReplays::handleConnected(const ServerInfo_User &userInfo)
{
    setRemoteEnabled(userInfo.user_level() & ServerInfo_User::IsRegistered);
}

/**
 * This is only responsible for handling the disconnect. The connect is already handled elsewhere
 */
void TabReplays::handleConnectionChanged(ClientStatus status)
{
    if (status == StatusDisconnected) {
        setRemoteEnabled(false);
    }
}

void TabReplays::setRemoteEnabled(bool enabled)
{
    aOpenRemoteReplay->setEnabled(enabled);
    aDownload->setEnabled(enabled);
    aKeep->setEnabled(enabled);
    aDeleteRemoteReplay->setEnabled(enabled);
    aGetReplayCode->setEnabled(enabled);
    aSubmitReplayCode->setEnabled(enabled);

    if (enabled) {
        serverDirView->refreshTree();
    } else {
        serverDirView->clearTree();
    }
}

void TabReplays::actLocalDoubleClick(const QModelIndex &curLeft)
{
    if (!localDirModel->isDir(curLeft)) {
        actOpenLocalReplay();
    }
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

void TabReplays::actRenameLocal()
{
    QModelIndexList curLefts = localDirView->selectionModel()->selectedRows();
    for (const auto &curLeft : curLefts) {
        const QFileInfo info = localDirModel->fileInfo(curLeft);

        const QString oldName = info.baseName();
        const QString title = info.isDir() ? tr("Rename local folder") : tr("Rename local file");

        bool ok;
        QString newName = QInputDialog::getText(this, title, tr("New name:"), QLineEdit::Normal, oldName, &ok);
        if (!ok) { // terminate all remaining selections if user cancels
            return;
        }
        if (newName.isEmpty() || oldName == newName) {
            continue;
        }

        QString newFileName = newName;
        if (!info.suffix().isEmpty()) {
            newFileName += "." + info.suffix();
        }
        const QString newFilePath = QFileInfo(info.dir(), newFileName).filePath();

        if (!QFile::rename(info.filePath(), newFilePath)) {
            QMessageBox::critical(this, tr("Error"), tr("Rename failed"));
        }
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

void TabReplays::actOpenReplaysFolder()
{
    QString dir = localDirModel->rootPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void TabReplays::actRemoteDoubleClick(const QModelIndex &curRight)
{
    if (serverDirView->getReplay(curRight)) {
        actOpenRemoteReplay();
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
        connect(pend, &PendingCommand::finished, this, &TabReplays::openRemoteReplayFinished);
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
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    while (!localDirModel->isDir(curLeft)) {
        curLeft = curLeft.parent();
    }

    for (const auto curRight : serverDirView->selectionModel()->selectedRows()) {
        downloadNodeAtIndex(curLeft, curRight);
    }
}

void TabReplays::downloadNodeAtIndex(const QModelIndex &curLeft, const QModelIndex &curRight)
{
    if (const auto replayMatch = serverDirView->getReplayMatch(curRight)) {
        // node at index is a folder
        const QString name =
            QString::number(replayMatch->game_id()) + "_" + QString::fromStdString(replayMatch->game_name());

        const auto dirIndex = curLeft.isValid() ? curLeft : localDirModel->index(localDirModel->rootPath());
        const auto newDirIndex = localDirModel->mkdir(dirIndex, name);

        int rows = serverDirView->model()->rowCount(curRight);
        for (int i = 0; i < rows; i++) {
            const auto childIndex = serverDirView->model()->index(i, 0, curRight);
            downloadNodeAtIndex(newDirIndex, childIndex);
        }
    } else if (const auto replay = serverDirView->getReplay(curRight)) {
        // node at index is a replay
        const QString dirPath = curLeft.isValid() ? localDirModel->filePath(curLeft) : localDirModel->rootPath();
        const QString filePath = dirPath + QString("/replay_%1.cor").arg(replay->replay_id());

        Command_ReplayDownload cmd;
        cmd.set_replay_id(replay->replay_id());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        pend->setExtraData(filePath);
        connect(pend, &PendingCommand::finished, this, &TabReplays::downloadFinished);
        client->sendCommand(pend);
    }
    // node at index was invalid
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
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning() << "failed to open" << filePath << "for writing after downloading replay";
        return;
    }
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
        connect(pend, &PendingCommand::finished, this, &TabReplays::keepRemoteReplayFinished);
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
        connect(pend, &PendingCommand::finished, this, &TabReplays::deleteRemoteReplayFinished);
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

void TabReplays::actGetReplayCode()
{
    const auto curRights = serverDirView->getSelectedReplayMatches();
    if (curRights.isEmpty()) {
        return;
    }

    for (const auto curRight : curRights) {
        Command_ReplayGetCode cmd;
        cmd.set_game_id(curRight->game_id());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &TabReplays::getReplayCodeFinished);
        client->sendCommand(pend);
    }
}

void TabReplays::getReplayCodeFinished(const Response &r, const CommandContainer & /*commandContainer*/)
{
    if (r.response_code() == Response::RespFunctionNotAllowed) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Failed to get code"));
        msgBox.setInformativeText(
            tr("Either this server does not support replay sharing, or does not permit replay sharing for you."));
        msgBox.exec();
        return;
    }

    if (r.response_code() != Response::RespOk) {
        QMessageBox::warning(this, tr("Failed"), tr("Could not get replay code"));
        return;
    }

    const Response_ReplayGetCode &resp = r.GetExtension(Response_ReplayGetCode::ext);
    QString code = QString::fromStdString(resp.replay_code());

    QMessageBox msgBox;
    msgBox.setText(tr("Replay Share Code"));
    msgBox.setInformativeText(
        tr("Others can use this code to add the replay to their list of remote replays:\n%1").arg(code));
    msgBox.setStandardButtons(QMessageBox::Ok);
    QPushButton *copyToClipboardButton = msgBox.addButton(tr("Copy to clipboard"), QMessageBox::ActionRole);
    connect(copyToClipboardButton, &QPushButton::clicked, this, [code] { QApplication::clipboard()->setText(code); });
    msgBox.setDefaultButton(copyToClipboardButton);
    msgBox.exec();
}

void TabReplays::actSubmitReplayCode()
{
    bool ok;
    QString code = QInputDialog::getText(this, tr("Look up replay by share code"), tr("Replay share code"),
                                         QLineEdit::Normal, "", &ok);

    if (!ok) {
        return;
    }

    Command_ReplaySubmitCode cmd;
    cmd.set_replay_code(code.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReplays::submitReplayCodeFinished);
    client->sendCommand(pend);
}

void TabReplays::submitReplayCodeFinished(const Response &r, const CommandContainer & /*commandContainer*/)
{
    switch (r.response_code()) {
        case Response::RespOk: {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("Replay code found"));
            msgBox.setInformativeText(tr("Replay was added, or you already had access to it."));
            msgBox.exec();
            break;
        }
        case Response::RespNameNotFound:
            QMessageBox::warning(this, tr("Failed"), tr("Replay code not found"));
            break;
        case Response::RespFunctionNotAllowed: {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Failed to submit code"));
            msgBox.setInformativeText(
                tr("Either this server does not support replay sharing, or does not permit replay sharing for you."));
            msgBox.exec();
            break;
        }
        default:
            QMessageBox::warning(this, tr("Failed"), tr("Unexpected error"));
            break;
    }
}

void TabReplays::replayAddedEventReceived(const Event_ReplayAdded &event)
{
    if (event.has_match_info()) {
        // 99.9% of events will have match info (Normal Workflow)
        serverDirView->addMatchInfo(event.match_info());
    } else {
        // When a Moderator force adds a replay or a user submits a replay code, we need to refresh their view
        serverDirView->refreshTree();
    }
}
