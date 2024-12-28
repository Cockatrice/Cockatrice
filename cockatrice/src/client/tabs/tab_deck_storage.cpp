#include "tab_deck_storage.h"

#include "../../deck/deck_loader.h"
#include "../../server/pending_command.h"
#include "../../server/remote/remote_decklist_tree_widget.h"
#include "../../settings/cache_settings.h"
#include "../game_logic/abstract_client.h"
#include "../get_text_with_max.h"
#include "decklist.h"
#include "pb/command_deck_del.pb.h"
#include "pb/command_deck_del_dir.pb.h"
#include "pb/command_deck_download.pb.h"
#include "pb/command_deck_new_dir.pb.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/response.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/response_deck_upload.pb.h"

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
#include <QVBoxLayout>

TabDeckStorage::TabDeckStorage(TabSupervisor *_tabSupervisor, AbstractClient *_client)
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
    connect(aOpenLocalDeck, SIGNAL(triggered()), this, SLOT(actOpenLocalDeck()));
    aUpload = new QAction(this);
    aUpload->setIcon(QPixmap("theme:icons/arrow_right_green"));
    connect(aUpload, SIGNAL(triggered()), this, SLOT(actUpload()));
    aNewLocalFolder = new QAction(this);
    aNewLocalFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(aNewLocalFolder, &QAction::triggered, this, &TabDeckStorage::actNewLocalFolder);
    aDeleteLocalDeck = new QAction(this);
    aDeleteLocalDeck->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteLocalDeck, SIGNAL(triggered()), this, SLOT(actDeleteLocalDeck()));

    aOpenDecksFolder = new QAction(this);
    aOpenDecksFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(aOpenDecksFolder, &QAction::triggered, this, &TabDeckStorage::actOpenDecksFolder);

    // Right side actions
    aOpenRemoteDeck = new QAction(this);
    aOpenRemoteDeck->setIcon(QPixmap("theme:icons/pencil"));
    connect(aOpenRemoteDeck, SIGNAL(triggered()), this, SLOT(actOpenRemoteDeck()));
    aDownload = new QAction(this);
    aDownload->setIcon(QPixmap("theme:icons/arrow_left_green"));
    connect(aDownload, SIGNAL(triggered()), this, SLOT(actDownload()));
    aNewFolder = new QAction(this);
    aNewFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(aNewFolder, SIGNAL(triggered()), this, SLOT(actNewFolder()));
    aDeleteRemoteDeck = new QAction(this);
    aDeleteRemoteDeck->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteRemoteDeck, SIGNAL(triggered()), this, SLOT(actDeleteRemoteDeck()));

    // Add actions to toolbars
    leftToolBar->addAction(aOpenLocalDeck);
    leftToolBar->addAction(aUpload);
    leftToolBar->addAction(aNewLocalFolder);
    leftToolBar->addAction(aDeleteLocalDeck);

    leftToolBar->addAction(aOpenDecksFolder);

    rightToolBar->addAction(aOpenRemoteDeck);
    rightToolBar->addAction(aDownload);
    rightToolBar->addAction(aNewFolder);
    rightToolBar->addAction(aDeleteRemoteDeck);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);
}

void TabDeckStorage::retranslateUi()
{
    leftGroupBox->setTitle(tr("Local file system"));
    rightGroupBox->setTitle(tr("Server deck storage"));

    aOpenLocalDeck->setText(tr("Open in deck editor"));
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

        DeckLoader deckLoader;
        if (!deckLoader.loadFromFile(filePath, DeckLoader::CockatriceFormat))
            continue;

        SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(filePath);

        emit openDeckEditor(&deckLoader);
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

    DeckLoader deck;
    if (!deck.loadFromFile(filePath, DeckLoader::CockatriceFormat)) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck file"));
        return;
    }

    if (deck.getName().isEmpty()) {
        bool ok;
        QString deckName =
            getTextWithMax(this, tr("Enter deck name"), tr("This decklist does not have a name.\nPlease enter a name:"),
                           QLineEdit::Normal, deckFileInfo.completeBaseName(), &ok);
        if (!ok)
            return;
        if (deckName.isEmpty())
            deckName = tr("Unnamed deck");
        deck.setName(deckName);
    } else {
        deck.setName(deck.getName().left(MAX_NAME_LENGTH));
    }

    QString deckString = deck.writeToString_Native();
    if (deckString.length() > MAX_FILE_LENGTH) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck file"));
        return;
    }

    Command_DeckUpload cmd;
    cmd.set_path(targetPath.toStdString());
    cmd.set_deck_list(deckString.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(uploadFinished(Response, CommandContainer)));
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
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(openRemoteDeckFinished(Response, CommandContainer)));
        client->sendCommand(pend);
    }
}

void TabDeckStorage::openRemoteDeckFinished(const Response &r, const CommandContainer &commandContainer)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    const Command_DeckDownload &cmd = commandContainer.session_command(0).GetExtension(Command_DeckDownload::ext);

    DeckLoader loader;
    if (!loader.loadFromRemote(QString::fromStdString(resp.deck()), cmd.deck_id()))
        return;

    emit openDeckEditor(&loader);
}

void TabDeckStorage::actDownload()
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

    for (const auto &curRight : serverDirView->getCurrentSelection()) {
        RemoteDeckList_TreeModel::FileNode *node = dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(curRight);
        if (!node)
            continue;

        QString filePath = dirPath + QString("/deck_%1.cod").arg(node->getId());

        Command_DeckDownload cmd;
        cmd.set_deck_id(node->getId());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        pend->setExtraData(filePath);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(downloadFinished(Response, CommandContainer, QVariant)));
        client->sendCommand(pend);
    }
}

void TabDeckStorage::downloadFinished(const Response &r,
                                      const CommandContainer & /*commandContainer*/,
                                      const QVariant &extraData)
{
    if (r.response_code() != Response::RespOk)
        return;

    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    QString filePath = extraData.toString();

    DeckLoader deck(QString::fromStdString(resp.deck()));
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
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(newFolderFinished(Response, CommandContainer)));
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
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(deleteFolderFinished(Response, CommandContainer)));
    } else {
        const auto *deckNode = dynamic_cast<const RemoteDeckList_TreeModel::FileNode *>(curRight);
        Command_DeckDel cmd;
        cmd.set_deck_id(deckNode->getId());
        pend = client->prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(deleteDeckFinished(Response, CommandContainer)));
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
