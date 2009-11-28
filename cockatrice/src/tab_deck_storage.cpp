#include <QtGui>
#include <QDebug>
#include "tab_deck_storage.h"
#include "remotedecklist_treewidget.h"
#include "client.h"
#include "decklist.h"
#include "protocol_items.h"

TabDeckStorage::TabDeckStorage(Client *_client)
	: Tab(), client(_client)
{
	localDirModel = new QFileSystemModel(this);
	QSettings settings;
	localDirModel->setRootPath(settings.value("paths/decks").toString());
	
	sortFilter = new QSortFilterProxyModel(this);
	sortFilter->setSourceModel(localDirModel);
	sortFilter->setDynamicSortFilter(true);
	
	localDirView = new QTreeView;
	localDirView->setModel(sortFilter);
	localDirView->setColumnHidden(1, true);
	localDirView->setRootIndex(sortFilter->mapFromSource(localDirModel->index(localDirModel->rootPath(), 0)));
	localDirView->setSortingEnabled(true);
	localDirView->header()->setResizeMode(QHeaderView::ResizeToContents);
	
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
	
	aUpload = new QAction(this);
	aUpload->setIcon(QIcon(":/resources/arrow_right_green.svg"));
	connect(aUpload, SIGNAL(triggered()), this, SLOT(actUpload()));
	aDownload = new QAction(this);
	aDownload->setIcon(QIcon(":/resources/arrow_left_green.svg"));
	connect(aDownload, SIGNAL(triggered()), this, SLOT(actDownload()));
	aNewFolder = new QAction(this);
	aNewFolder->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
	connect(aNewFolder, SIGNAL(triggered()), this, SLOT(actNewFolder()));
	aDelete = new QAction(this);
	aDelete->setIcon(QIcon(":/resources/remove_row.svg"));
	connect(aDelete, SIGNAL(triggered()), this, SLOT(actDelete()));
	
	leftToolBar->addAction(aUpload);
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
	
	aUpload->setText(tr("Upload deck"));
	aDownload->setText(tr("Download deck"));
	aNewFolder->setText(tr("New folder"));
	aDelete->setText(tr("Delete"));
	
	serverDirView->retranslateUi();
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

	QString targetPath;
	QTreeWidgetItem *curRight = serverDirView->currentItem();
	while ((curRight != 0) && (curRight->type() != TWIFolderType))
		curRight = curRight->parent();
	if (curRight)
		targetPath = curRight->data(0, Qt::UserRole).toString();

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
	delete cmd->getDeck();

	QTreeWidgetItemIterator it(serverDirView);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == cmd->getPath()) {
			serverDirView->addFileToTree(resp->getFile(), *it);
			break;
		}
		++it;
	}
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

	QTreeWidgetItem *curRight = serverDirView->currentItem();
	if ((!curRight) || (curRight->type() != TWIDeckType))
		return;
	filePath += "/" + curRight->data(1, Qt::DisplayRole).toString() + ".cod";

	Command_DeckDownload *command = new Command_DeckDownload(curRight->data(1, Qt::DisplayRole).toInt());
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
	QTreeWidgetItem *curRight = serverDirView->currentItem();
	while ((curRight != 0) && (curRight->type() != TWIFolderType))
		curRight = curRight->parent();
	if (curRight)
		targetPath = curRight->data(0, Qt::UserRole).toString();

	Command_DeckNewDir *command = new Command_DeckNewDir(targetPath, folderName);
	connect(command, SIGNAL(finished(ResponseCode)), this, SLOT(newFolderFinished(ResponseCode)));
	client->sendCommand(command);
}

void TabDeckStorage::newFolderFinished(ResponseCode resp)
{
	if (resp != RespOk)
		return;

	Command_DeckNewDir *cmd = static_cast<Command_DeckNewDir *>(sender());

	qDebug() << cmd->getPath() << cmd->getDirName();
	QTreeWidgetItemIterator it(serverDirView);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == cmd->getPath()) {
			QFileIconProvider fip;
			QTreeWidgetItem *newItem = new QTreeWidgetItem(TWIFolderType);
			newItem->setIcon(0, fip.icon(QFileIconProvider::Folder));
			newItem->setText(0, cmd->getDirName());
			newItem->setData(0, Qt::UserRole, cmd->getPath() + "/" + cmd->getDirName());
			(*it)->addChild(newItem);
			break;
		}
		++it;
	}
}

void TabDeckStorage::actDelete()
{
	Command *command;
	QTreeWidgetItem *curRight = serverDirView->currentItem();
	if (curRight->type() == TWIFolderType) {
		if (curRight->data(0, Qt::UserRole).toString().isEmpty())
			return;
		command = new Command_DeckDelDir(curRight->data(0, Qt::UserRole).toString());
	} else
		command = new Command_DeckDel(curRight->data(1, Qt::DisplayRole).toInt());
	connect(command, SIGNAL(finished(ResponseCode)), this, SLOT(deleteFinished(ResponseCode)));
	client->sendCommand(command);
}

void TabDeckStorage::deleteFinished(ResponseCode resp)
{
	if (resp != RespOk)
		return;

	QTreeWidgetItem *toDelete = 0;
	QTreeWidgetItemIterator it(serverDirView);
	Command_DeckDelDir *cmdDelDir = qobject_cast<Command_DeckDelDir *>(sender());
	if (cmdDelDir) {
		while (*it) {
			if ((*it)->data(0, Qt::UserRole).toString() == cmdDelDir->getPath()) {
				toDelete = *it;
				break;
			}
			++it;
		}
	} else {
		Command_DeckDel *cmdDel = qobject_cast<Command_DeckDel *>(sender());
		while (*it) {
			if ((*it)->data(1, Qt::DisplayRole).toInt() == cmdDel->getDeckId()) {
				toDelete = *it;
				break;
			}
			++it;
		}
	}
	if (toDelete)
		delete toDelete;
}
