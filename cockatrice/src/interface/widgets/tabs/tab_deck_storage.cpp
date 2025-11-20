#include "tab_deck_storage.h"

#include "../../../client/settings/cache_settings.h"
#include "../../deck_loader/deck_loader.h"
#include "../interface/widgets/server/remote/remote_decklist_tree_widget.h"
#include "../interface/widgets/utility/get_text_with_max.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
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
#include <QVBoxLayout>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/pb/command_deck_del.pb.h>
#include <libcockatrice/protocol/pb/command_deck_del_dir.pb.h>
#include <libcockatrice/protocol/pb/command_deck_download.pb.h>
#include <libcockatrice/protocol/pb/command_deck_new_dir.pb.h>
#include <libcockatrice/protocol/pb/command_deck_upload.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pb/response_deck_upload.pb.h>
#include <libcockatrice/protocol/pending_command.h>

TabDeckStorage::TabDeckStorage(TabSupervisor *_tabSupervisor,
                               AbstractClient *_client,
                               const ServerInfo_User *currentUserInfo)
    : Tab(_tabSupervisor), client(_client)
{
    localDirModel = new QFileSystemModel(this);
    localDirModel->setRootPath(SettingsCache::instance().getDeckPath());
    localDirModel->sort(0, Qt::AscendingOrder);

    localDirView = new QTreeView;
    localDirView->setModel(localDirModel);
    localDirView->setColumnHidden(1, true);
    localDirView->setRootIndex(localDirModel->index(localDirModel->rootPath(), 0));
    localDirView->setSortingEnabled(true);
    localDirView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    localDirView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    localDirView->header()->setSortIndicator(0, Qt::AscendingOrder);

    connect(localDirView, &QTreeView::doubleClicked, this, &TabDeckStorage::actLocalDoubleClick);

    // Left side layout
    /* put an invisible dummy QToolBar in the leftmost column so that the main toolbar is centered.
     * Really ugly workaround, but I couldn't figure out the proper way to make it centered */
    QToolBar *dummyToolBar = new QToolBar(this);
    QSizePolicy sizePolicy = dummyToolBar->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    dummyToolBar->setSizePolicy(sizePolicy);
    dummyToolBar->setVisible(false);

    leftToolBar = new QToolBar(this);
    leftToolBar->setOrientation(Qt::Horizontal);
    leftToolBar->setIconSize(QSize(32, 32));

    QToolBar *leftRightmostToolBar = new QToolBar(this);
    leftRightmostToolBar->setOrientation(Qt::Horizontal);
    leftRightmostToolBar->setIconSize(QSize(32, 32));

    QGridLayout *leftToolBarLayout = new QGridLayout;
    leftToolBarLayout->addWidget(dummyToolBar, 0, 0, Qt::AlignLeft);
    leftToolBarLayout->addWidget(leftToolBar, 0, 1, Qt::AlignHCenter);
    leftToolBarLayout->addWidget(leftRightmostToolBar, 0, 2, Qt::AlignRight);

    QVBoxLayout *leftVbox = new QVBoxLayout;
    leftVbox->addWidget(localDirView);
    leftVbox->addLayout(leftToolBarLayout);
    leftGroupBox = new QGroupBox;
    leftGroupBox->setLayout(leftVbox);

    // Right side layout
    rightToolBar = new QToolBar;
    rightToolBar->setOrientation(Qt::Horizontal);
    rightToolBar->setIconSize(QSize(32, 32));
    QHBoxLayout *rightToolBarLayout = new QHBoxLayout;
    rightToolBarLayout->addStretch();
    rightToolBarLayout->addWidget(rightToolBar);
    rightToolBarLayout->addStretch();

    serverDirView = new RemoteDeckList_TreeWidget(client);

    connect(serverDirView, &QTreeView::doubleClicked, this, &TabDeckStorage::actRemoteDoubleClick);

    QVBoxLayout *rightVbox = new QVBoxLayout;
    rightVbox->addWidget(serverDirView);
    rightVbox->addLayout(rightToolBarLayout);
    rightGroupBox = new QGroupBox;
    rightGroupBox->setLayout(rightVbox);

    // combine layouts
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(leftGroupBox);
    hbox->addWidget(rightGroupBox);

    // Left side actions
    aOpenLocalDeck = new QAction(this);
    aOpenLocalDeck->setIcon(QPixmap("theme:icons/pencil"));
    connect(aOpenLocalDeck, &QAction::triggered, this, &TabDeckStorage::actOpenLocalDeck);
    aRenameLocal = new QAction(this);
    aRenameLocal->setIcon(QPixmap("theme:icons/rename"));
    connect(aRenameLocal, &QAction::triggered, this, &TabDeckStorage::actRenameLocal);
    aUpload = new QAction(this);
    aUpload->setIcon(QPixmap("theme:icons/arrow_right_green"));
    connect(aUpload, &QAction::triggered, this, &TabDeckStorage::actUpload);
    aNewLocalFolder = new QAction(this);
    aNewLocalFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(aNewLocalFolder, &QAction::triggered, this, &TabDeckStorage::actNewLocalFolder);
    aDeleteLocalDeck = new QAction(this);
    aDeleteLocalDeck->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteLocalDeck, &QAction::triggered, this, &TabDeckStorage::actDeleteLocalDeck);

    aOpenDecksFolder = new QAction(this);
    aOpenDecksFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(aOpenDecksFolder, &QAction::triggered, this, &TabDeckStorage::actOpenDecksFolder);

    // Right side actions
    aOpenRemoteDeck = new QAction(this);
    aOpenRemoteDeck->setIcon(QPixmap("theme:icons/pencil"));
    connect(aOpenRemoteDeck, &QAction::triggered, this, &TabDeckStorage::actOpenRemoteDeck);
    aDownload = new QAction(this);
    aDownload->setIcon(QPixmap("theme:icons/arrow_left_green"));
    connect(aDownload, &QAction::triggered, this, &TabDeckStorage::actDownload);
    aNewFolder = new QAction(this);
    aNewFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(aNewFolder, &QAction::triggered, this, &TabDeckStorage::actNewFolder);
    aDeleteRemoteDeck = new QAction(this);
    aDeleteRemoteDeck->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteRemoteDeck, &QAction::triggered, this, &TabDeckStorage::actDeleteRemoteDeck);

    // Add actions to toolbars
    leftToolBar->addAction(aOpenLocalDeck);
    leftToolBar->addAction(aRenameLocal);
    leftToolBar->addAction(aUpload);
    leftToolBar->addAction(aNewLocalFolder);
    leftToolBar->addAction(aDeleteLocalDeck);

    leftRightmostToolBar->addAction(aOpenDecksFolder);

    rightToolBar->addAction(aOpenRemoteDeck);
    rightToolBar->addAction(aDownload);
    rightToolBar->addAction(aNewFolder);
    rightToolBar->addAction(aDeleteRemoteDeck);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);

    connect(client, &AbstractClient::userInfoChanged, this, &TabDeckStorage::handleConnected);
    connect(client, &AbstractClient::statusChanged, this, &TabDeckStorage::handleConnectionChanged);
    setRemoteEnabled(currentUserInfo && currentUserInfo->user_level() & ServerInfo_User::IsRegistered);
}

void TabDeckStorage::retranslateUi()
{
    leftGroupBox->setTitle(tr("Local file system"));
    rightGroupBox->setTitle(tr("Server deck storage"));

    aOpenLocalDeck->setText(tr("Open in deck editor"));
    aRenameLocal->setText(tr("Rename deck or folder"));
    aUpload->setText(tr("Upload deck"));
    aOpenRemoteDeck->setText(tr("Open in deck editor"));
    aDownload->setText(tr("Download deck"));
    aNewLocalFolder->setText(tr("New folder"));
    aNewFolder->setText(tr("New folder"));
    aDeleteLocalDeck->setText(tr("Delete"));
    aDeleteRemoteDeck->setText(tr("Delete"));
    aOpenDecksFolder->setText(tr("Open decks folder"));
}

QString TabDeckStorage::getTargetPath() const
{
    RemoteDeckList_TreeModel::Node *curRight = serverDirView->getCurrentItem();
    if (curRight == nullptr)
        return {};
    auto *dir = dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight);
    if (dir == nullptr) {
        dir = dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight->getParent());
        if (dir != nullptr) {
            return dir->getPath();
        } else {
            return {};
        }
    } else {
        return dir->getPath();
    }
}

void TabDeckStorage::handleConnected(const ServerInfo_User &userInfo)
{
    setRemoteEnabled(userInfo.user_level() & ServerInfo_User::IsRegistered);
}

/**
 * This is only responsible for handling the disconnect. The connect is already handled elsewhere
 */
void TabDeckStorage::handleConnectionChanged(ClientStatus status)
{
    if (status == StatusDisconnected) {
        setRemoteEnabled(false);
    }
}

void TabDeckStorage::setRemoteEnabled(bool enabled)
{
    aUpload->setEnabled(enabled);
    aOpenRemoteDeck->setEnabled(enabled);
    aDownload->setEnabled(enabled);
    aNewFolder->setEnabled(enabled);
    aDeleteRemoteDeck->setEnabled(enabled);

    if (enabled) {
        serverDirView->refreshTree();
    } else {
        serverDirView->clearTree();
    }
}

void TabDeckStorage::actLocalDoubleClick(const QModelIndex &curLeft)
{
    if (!localDirModel->isDir(curLeft)) {
        actOpenLocalDeck();
    }
}

void TabDeckStorage::actOpenLocalDeck()
{
    QModelIndexList curLefts = localDirView->selectionModel()->selectedRows();
    for (const auto &curLeft : curLefts) {
        if (localDirModel->isDir(curLeft))
            continue;
        QString filePath = localDirModel->filePath(curLeft);

        auto deckLoader = new DeckLoader(this);
        if (!deckLoader->loadFromFile(filePath, DeckLoader::CockatriceFormat, true))
            continue;

        emit openDeckEditor(deckLoader);
    }
}

void TabDeckStorage::actRenameLocal()
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

void TabDeckStorage::actUpload()
{
    QModelIndexList curLefts = localDirView->selectionModel()->selectedRows();
    if (curLefts.isEmpty()) {
        return;
    }

    QString targetPath = getTargetPath();
    if (targetPath.length() > MAX_NAME_LENGTH) {
        qCritical() << "target path to upload to is too long" << targetPath;
        return;
    }

    for (const auto &curLeft : curLefts) {
        if (localDirModel->isDir(curLeft)) {
            continue;
        }

        QString filePath = localDirModel->filePath(curLeft);
        uploadDeck(filePath, targetPath);
    }
}

void TabDeckStorage::uploadDeck(const QString &filePath, const QString &targetPath)
{
    QFile deckFile(filePath);
    QFileInfo deckFileInfo(deckFile);

    DeckLoader deck(this);
    if (!deck.loadFromFile(filePath, DeckLoader::CockatriceFormat)) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck file"));
        return;
    }

    if (deck.getDeckList()->getName().isEmpty()) {
        bool ok;
        QString deckName =
            getTextWithMax(this, tr("Enter deck name"), tr("This decklist does not have a name.\nPlease enter a name:"),
                           QLineEdit::Normal, deckFileInfo.completeBaseName(), &ok);
        if (!ok)
            return;
        if (deckName.isEmpty())
            deckName = tr("Unnamed deck");
        deck.getDeckList()->setName(deckName);
    } else {
        deck.getDeckList()->setName(deck.getDeckList()->getName().left(MAX_NAME_LENGTH));
    }

    QString deckString = deck.getDeckList()->writeToString_Native();
    if (deckString.length() > MAX_FILE_LENGTH) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck file"));
        return;
    }

    Command_DeckUpload cmd;
    cmd.set_path(targetPath.toStdString());
    cmd.set_deck_list(deckString.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabDeckStorage::uploadFinished);
    client->sendCommand(pend);
}

void TabDeckStorage::uploadFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk) {
        qCritical() << "failed to upload deck:" << r.response_code();
        QMessageBox::critical(this, tr("Error"), tr("Failed to upload deck to server"));
        return;
    }

    const Response_DeckUpload &resp = r.GetExtension(Response_DeckUpload::ext);
    const Command_DeckUpload &cmd = commandContainer.session_command(0).GetExtension(Command_DeckUpload::ext);

    serverDirView->addFileToTree(resp.new_file(), serverDirView->getNodeByPath(QString::fromStdString(cmd.path())));
}

void TabDeckStorage::actNewLocalFolder()
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

void TabDeckStorage::actDeleteLocalDeck()
{
    const QModelIndexList curLefts = localDirView->selectionModel()->selectedRows();

    if (curLefts.isEmpty()) {
        return;
    }

    if (QMessageBox::warning(this, tr("Delete local file"), tr("Are you sure you want to delete the selected files?"),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    for (const auto &curLeft : curLefts) {
        if (curLeft.isValid()) {
            localDirModel->remove(curLeft);
        }
    }
}

void TabDeckStorage::actOpenDecksFolder()
{
    QString dir = localDirModel->rootPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void TabDeckStorage::actRemoteDoubleClick(const QModelIndex &curRight)
{
    if (dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(serverDirView->getNode(curRight))) {
        actOpenRemoteDeck();
    }
}

void TabDeckStorage::actOpenRemoteDeck()
{
    for (const auto &curRight : serverDirView->getCurrentSelection()) {
        RemoteDeckList_TreeModel::FileNode *node = dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(curRight);
        if (!node)
            continue;

        Command_DeckDownload cmd;
        cmd.set_deck_id(node->getId());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &TabDeckStorage::openRemoteDeckFinished);
        client->sendCommand(pend);
    }
}

void TabDeckStorage::openRemoteDeckFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    const Command_DeckDownload &cmd = commandContainer.session_command(0).GetExtension(Command_DeckDownload::ext);

    DeckLoader loader(this);
    if (!loader.loadFromRemote(QString::fromStdString(resp.deck()), cmd.deck_id()))
        return;

    emit openDeckEditor(&loader);
}

void TabDeckStorage::actDownload()
{
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    while (!localDirModel->isDir(curLeft)) {
        curLeft = curLeft.parent();
    }

    for (const auto curRight : serverDirView->selectionModel()->selectedRows()) {
        downloadNodeAtIndex(curLeft, curRight);
    }
}

void TabDeckStorage::downloadNodeAtIndex(const QModelIndex &curLeft, const QModelIndex &curRight)
{
    auto node = serverDirView->getNode(curRight);
    if (const auto dirNode = dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(node)) {
        // node at index is a folder
        const QString name = dirNode->getName();

        const auto dirIndex = curLeft.isValid() ? curLeft : localDirModel->index(localDirModel->rootPath());
        const auto newDirIndex = localDirModel->mkdir(dirIndex, name);

        int rows = serverDirView->model()->rowCount(curRight);
        for (int i = 0; i < rows; i++) {
            const auto childIndex = serverDirView->model()->index(i, 0, curRight);
            downloadNodeAtIndex(newDirIndex, childIndex);
        }
    } else if (const auto fileNode = dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(node)) {
        // node at index is a deck
        const QString dirPath = curLeft.isValid() ? localDirModel->filePath(curLeft) : localDirModel->rootPath();
        const QString filePath = dirPath + QString("/deck_%1.cod").arg(fileNode->getId());

        Command_DeckDownload cmd;
        cmd.set_deck_id(fileNode->getId());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        pend->setExtraData(filePath);
        connect(pend, &PendingCommand::finished, this, &TabDeckStorage::downloadFinished);
        client->sendCommand(pend);
    }
    // node at index is invalid
}

void TabDeckStorage::downloadFinished(const Response &r,
                                      const CommandContainer & /*commandContainer*/,
                                      const QVariant &extraData)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    QString filePath = extraData.toString();

    DeckLoader deck(this, new DeckList(QString::fromStdString(resp.deck())));
    deck.saveToFile(filePath, DeckLoader::CockatriceFormat);
}

void TabDeckStorage::actNewFolder()
{
    QString targetPath = getTargetPath();
    int max_length = MAX_NAME_LENGTH - targetPath.length() - 1; // generated length would be path + / + name

    if (max_length < 1) // can't create path that's short enough
        return;

    QString folderName = getTextWithMax(this, tr("New folder"), tr("Name of new folder:"), max_length);
    if (folderName.isEmpty())
        return;

    // '/' isn't a valid filename character on *nix so we're choosing to replace it with a different arbitrary
    // character.
    std::string folder = folderName.toStdString();
    std::replace(folder.begin(), folder.end(), '/', '-');

    Command_DeckNewDir cmd;
    cmd.set_path(targetPath.toStdString());
    cmd.set_dir_name(folder);

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabDeckStorage::newFolderFinished);
    client->sendCommand(pend);
}

void TabDeckStorage::newFolderFinished(const Response &response, const CommandContainer &commandContainer)
{
    if (response.response_code() != Response::RespOk)
        return;

    const Command_DeckNewDir &cmd = commandContainer.session_command(0).GetExtension(Command_DeckNewDir::ext);
    serverDirView->addFolderToTree(QString::fromStdString(cmd.dir_name()),
                                   serverDirView->getNodeByPath(QString::fromStdString(cmd.path())));
}

void TabDeckStorage::actDeleteRemoteDeck()
{
    auto curRights = serverDirView->getCurrentSelection();

    if (curRights.isEmpty()) {
        return;
    }

    if (QMessageBox::warning(this, tr("Delete remote decks"), tr("Are you sure you want to delete the selected decks?"),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    for (const auto &curRight : curRights) {
        deleteRemoteDeck(curRight);
    }
}

void TabDeckStorage::deleteRemoteDeck(const RemoteDeckList_TreeModel::Node *curRight)
{
    if (!curRight) {
        return;
    }

    PendingCommand *pend;
    if (const auto *dir = dynamic_cast<const RemoteDeckList_TreeModel::DirectoryNode *>(curRight)) {
        QString targetPath = dir->getPath();
        if (targetPath.isEmpty())
            return;
        if (targetPath.length() > MAX_NAME_LENGTH) {
            qCritical() << "target path to delete is too long" << targetPath;
            return;
        }
        Command_DeckDelDir cmd;
        cmd.set_path(targetPath.toStdString());
        pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &TabDeckStorage::deleteFolderFinished);
    } else {
        const auto *deckNode = dynamic_cast<const RemoteDeckList_TreeModel::FileNode *>(curRight);
        Command_DeckDel cmd;
        cmd.set_deck_id(deckNode->getId());
        pend = client->prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &TabDeckStorage::deleteDeckFinished);
    }

    client->sendCommand(pend);
}

void TabDeckStorage::deleteDeckFinished(const Response &response, const CommandContainer &commandContainer)
{
    if (response.response_code() != Response::RespOk)
        return;

    const Command_DeckDel &cmd = commandContainer.session_command(0).GetExtension(Command_DeckDel::ext);
    RemoteDeckList_TreeModel::Node *toDelete = serverDirView->getNodeById(cmd.deck_id());
    if (toDelete)
        serverDirView->removeNode(toDelete);
}

void TabDeckStorage::deleteFolderFinished(const Response &response, const CommandContainer &commandContainer)
{
    if (response.response_code() != Response::RespOk)
        return;

    const Command_DeckDelDir &cmd = commandContainer.session_command(0).GetExtension(Command_DeckDelDir::ext);
    RemoteDeckList_TreeModel::Node *toDelete = serverDirView->getNodeByPath(QString::fromStdString(cmd.path()));
    if (toDelete)
        serverDirView->removeNode(toDelete);
}
