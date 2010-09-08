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
#include "tab_deck_storage.h"
#include "remotedecklist_treewidget.h"
#include "abstractclient.h"
#include "decklist.h"
#include "protocol_items.h"
#include "window_deckeditor.h"
#include "settingscache.h"

TabDeckStorage::TabDeckStorage(AbstractClient *_client)
	: Tab(), client(_client)
{
	localDirModel = new QFileSystemModel(this);
	localDirModel->setRootPath(settingsCache->getDeckPath());
	
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

	serverDirView = new RemoteDeckList_TreeWidget(client);

	QVBoxLayout *rightVbox = new QVBoxLayout;
	rightVbox->addWidget(serverDirView);
	rightVbox->addLayout(rightToolBarLayout);
	rightGroupBox = new QGroupBox;
	rightGroupBox->setLayout(rightVbox);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(leftGroupBox);
	hbox->addWidget(rightGroupBox);
	
	aOpenLocalDeck = new QAction(this);
	aOpenLocalDeck->setIcon(QIcon(":/resources/pencil.svg"));
	connect(aOpenLocalDeck, SIGNAL(triggered()), this, SLOT(actOpenLocalDeck()));
	aUpload = new QAction(this);
	aUpload->setIcon(QIcon(":/resources/arrow_right_green.svg"));
	connect(aUpload, SIGNAL(triggered()), this, SLOT(actUpload()));
	aOpenRemoteDeck = new QAction(this);
	aOpenRemoteDeck->setIcon(QIcon(":/resources/pencil.svg"));
	connect(aOpenRemoteDeck, SIGNAL(triggered()), this, SLOT(actOpenRemoteDeck()));
	aDownload = new QAction(this);
	aDownload->setIcon(QIcon(":/resources/arrow_left_green.svg"));
	connect(aDownload, SIGNAL(triggered()), this, SLOT(actDownload()));
	aNewFolder = new QAction(this);
	aNewFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
	connect(aNewFolder, SIGNAL(triggered()), this, SLOT(actNewFolder()));
	aDelete = new QAction(this);
	aDelete->setIcon(QIcon(":/resources/remove_row.svg"));
	connect(aDelete, SIGNAL(triggered()), this, SLOT(actDelete()));
	
	leftToolBar->addAction(aOpenLocalDeck);
	leftToolBar->addAction(aUpload);
	rightToolBar->addAction(aOpenRemoteDeck);
	rightToolBar->addAction(aDownload);
	rightToolBar->addAction(aNewFolder);
	rightToolBar->addAction(aDelete);
	
	retranslateUi();
	setLayout(hbox);
}

void TabDeckStorage::retranslateUi()
{
	leftGroupBox->setTitle(tr("Local file system"));
	rightGroupBox->setTitle(tr("Server deck storage"));
	
	aOpenLocalDeck->setText(tr("Open in deck editor"));
	aUpload->setText(tr("Upload deck"));
	aOpenRemoteDeck->setText(tr("Open in deck editor"));
	aDownload->setText(tr("Download deck"));
	aNewFolder->setText(tr("New folder"));
	aDelete->setText(tr("Delete"));
}

void TabDeckStorage::actOpenLocalDeck()
{
	QModelIndex curLeft = sortFilter->mapToSource(localDirView->selectionModel()->currentIndex());
	if (localDirModel->isDir(curLeft))
		return;
	QString filePath = localDirModel->filePath(curLeft);
	DeckList *deck = new DeckList;
	if (!deck->loadFromFile(filePath, DeckList::CockatriceFormat))
		return;
	
	WndDeckEditor *deckEditor = new WndDeckEditor;
	deckEditor->setDeck(deck, filePath, DeckList::CockatriceFormat);
	deckEditor->show();
}

void TabDeckStorage::actUpload()
{
	QModelIndex curLeft = sortFilter->mapToSource(localDirView->selectionModel()->currentIndex());
	if (localDirModel->isDir(curLeft))
		return;
	QString filePath = localDirModel->filePath(curLeft);
	DeckList *deck = new DeckList;
	if (!deck->loadFromFile(filePath, DeckList::CockatriceFormat))
		return;
	if (deck->getName().isEmpty()) {
		bool ok;
		QString deckName = QInputDialog::getText(this, tr("Enter deck name"), tr("This decklist does not have a name.\nPlease enter a name:"), QLineEdit::Normal, tr("Unnamed deck"), &ok);
		if (!ok)
			return;
		if (deckName.isEmpty())
			deckName = tr("Unnamed deck");
		deck->setName(deckName);
	}

	QString targetPath;
	RemoteDeckList_TreeModel::Node *curRight = serverDirView->getCurrentItem();
	if (!curRight)
		return;
	if (!dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight))
		curRight = curRight->getParent();
	targetPath = dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight)->getPath();

	Command_DeckUpload *command = new Command_DeckUpload(deck, targetPath);
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(uploadFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void TabDeckStorage::uploadFinished(ProtocolResponse *r)
{
	Response_DeckUpload *resp = qobject_cast<Response_DeckUpload *>(r);
	if (!resp)
		return;
	Command_DeckUpload *cmd = static_cast<Command_DeckUpload *>(sender());

	serverDirView->addFileToTree(resp->getFile(), serverDirView->getNodeByPath(cmd->getPath()));
}

void TabDeckStorage::actOpenRemoteDeck()
{
	RemoteDeckList_TreeModel::FileNode *curRight = dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(serverDirView->getCurrentItem());
	if (!curRight)
		return;

	Command_DeckDownload *command = new Command_DeckDownload(curRight->getId());
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(openRemoteDeckFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void TabDeckStorage::openRemoteDeckFinished(ProtocolResponse *r)
{
	Response_DeckDownload *resp = qobject_cast<Response_DeckDownload *>(r);
	if (!resp)
		return;
	
	WndDeckEditor *deckEditor = new WndDeckEditor;
	deckEditor->setDeck(new DeckList(resp->getDeck()));
	deckEditor->show();
}

void TabDeckStorage::actDownload()
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

	RemoteDeckList_TreeModel::FileNode *curRight = dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(serverDirView->getCurrentItem());
	if (!curRight)
		return;
	filePath += QString("/deck_%1.cod").arg(curRight->getId());

	Command_DeckDownload *command = new Command_DeckDownload(curRight->getId());
	command->setExtraData(filePath);
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(downloadFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void TabDeckStorage::downloadFinished(ProtocolResponse *r)
{
	Response_DeckDownload *resp = qobject_cast<Response_DeckDownload *>(r);
	if (!resp)
		return;
	Command_DeckDownload *cmd = static_cast<Command_DeckDownload *>(sender());

	QString filePath = cmd->getExtraData().toString();
	resp->getDeck()->saveToFile(filePath, DeckList::CockatriceFormat);
}

void TabDeckStorage::actNewFolder()
{
	QString folderName = QInputDialog::getText(this, tr("New folder"), tr("Name of new folder:"));
	if (folderName.isEmpty())
		return;

	QString targetPath;
	RemoteDeckList_TreeModel::Node *curRight = serverDirView->getCurrentItem();
	if (!curRight)
		return;
	if (!dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight))
		curRight = curRight->getParent();
	RemoteDeckList_TreeModel::DirectoryNode *dir = dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight);
	targetPath = dir->getPath();

	Command_DeckNewDir *command = new Command_DeckNewDir(targetPath, folderName);
	connect(command, SIGNAL(finished(ResponseCode)), this, SLOT(newFolderFinished(ResponseCode)));
	client->sendCommand(command);
}

void TabDeckStorage::newFolderFinished(ResponseCode resp)
{
	if (resp != RespOk)
		return;

	Command_DeckNewDir *cmd = static_cast<Command_DeckNewDir *>(sender());
	serverDirView->addFolderToTree(cmd->getDirName(), serverDirView->getNodeByPath(cmd->getPath()));
}

void TabDeckStorage::actDelete()
{
	Command *command;
	RemoteDeckList_TreeModel::Node *curRight = serverDirView->getCurrentItem();
	if (!curRight)
		return;
	RemoteDeckList_TreeModel::DirectoryNode *dir = dynamic_cast<RemoteDeckList_TreeModel::DirectoryNode *>(curRight);
	if (dir) {
		QString path = dir->getPath();
		if (path.isEmpty())
			return;
		command = new Command_DeckDelDir(path);
	} else
		command = new Command_DeckDel(dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(curRight)->getId());
	connect(command, SIGNAL(finished(ResponseCode)), this, SLOT(deleteFinished(ResponseCode)));
	client->sendCommand(command);
}

void TabDeckStorage::deleteFinished(ResponseCode resp)
{
	if (resp != RespOk)
		return;

	RemoteDeckList_TreeModel::Node *toDelete = 0;
	Command_DeckDelDir *cmdDelDir = qobject_cast<Command_DeckDelDir *>(sender());
	if (cmdDelDir)
		toDelete = serverDirView->getNodeByPath(cmdDelDir->getPath());
	else
		toDelete = serverDirView->getNodeById(static_cast<Command_DeckDel *>(sender())->getDeckId());
	
	if (toDelete)
		serverDirView->removeNode(toDelete);
}
