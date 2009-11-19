#include <QtGui>
#include <QDebug>
#include "tab_deck_storage.h"
#include "client.h"
#include "decklist.h"
#include "protocol_items.h"

enum { TWIFolderType = QTreeWidgetItem::UserType + 1, TWIDeckType = QTreeWidgetItem::UserType + 2 };

TabDeckStorage::TabDeckStorage(Client *_client)
	: QWidget(), client(_client)
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
	
	QVBoxLayout *leftVbox = new QVBoxLayout;
	leftVbox->addWidget(localDirView);
	leftGroupBox = new QGroupBox;
	leftGroupBox->setLayout(leftVbox);
	
	toolBar = new QToolBar;
	toolBar->setOrientation(Qt::Vertical);
	toolBar->setIconSize(QSize(24, 24));
	
	serverDirView = new QTreeWidget;
	serverDirView->header()->setResizeMode(QHeaderView::ResizeToContents);
	serverDirView->setColumnCount(3);

	QVBoxLayout *rightVbox = new QVBoxLayout;
	rightVbox->addWidget(serverDirView);
	rightGroupBox = new QGroupBox;
	rightGroupBox->setLayout(rightVbox);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(leftGroupBox);
	hbox->addWidget(toolBar);
	hbox->addWidget(rightGroupBox);
	
	aUpload = new QAction(this);
	aUpload->setIcon(QIcon(":/resources/arrow_right_green.svg"));
	connect(aUpload, SIGNAL(triggered()), this, SLOT(actUpload()));
	aDownload = new QAction(this);
	aDownload->setIcon(QIcon(":/resources/arrow_left_green.svg"));
	connect(aDownload, SIGNAL(triggered()), this, SLOT(actDownload()));
	aNewFolder = new QAction(this);
	connect(aNewFolder, SIGNAL(triggered()), this, SLOT(actNewFolder()));
	aDelete = new QAction(this);
	aDelete->setIcon(QIcon(":/resources/remove_row.svg"));
	connect(aDelete, SIGNAL(triggered()), this, SLOT(actDelete()));
	
	toolBar->addAction(aUpload);
	toolBar->addAction(aDownload);
	toolBar->addAction(aNewFolder);
	toolBar->addAction(aDelete);
	
	retranslateUi();
	setLayout(hbox);

	refreshServerList();
}

void TabDeckStorage::retranslateUi()
{
	leftGroupBox->setTitle(tr("Local file system"));
	rightGroupBox->setTitle(tr("Server deck storage"));
	
	aUpload->setText(tr("Upload deck"));
	aDownload->setText(tr("Download deck"));
	aNewFolder->setText(tr("New folder"));
	aDelete->setText(tr("Delete"));

	QTreeWidgetItem *header = serverDirView->headerItem();
	header->setText(0, tr("Name"));
	header->setText(1, tr("ID"));
	header->setText(2, tr("Upload time"));
	header->setTextAlignment(1, Qt::AlignRight);
}

void TabDeckStorage::refreshServerList()
{
	Command_DeckList *command = new Command_DeckList;
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(deckListFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void TabDeckStorage::populateDeckList(Response_DeckList::Directory *folder, QTreeWidgetItem *parent)
{
	QFileIconProvider fip;
	QTreeWidgetItem *newItem = new QTreeWidgetItem(TWIFolderType);
	newItem->setIcon(0, fip.icon(QFileIconProvider::Folder));
	newItem->setText(0, parent ? folder->getName() : "/");
	QString parentPath;
	if (parent) {
		parent->addChild(newItem);
		parentPath = parent->data(0, Qt::UserRole).toString();
	} else
		serverDirView->addTopLevelItem(newItem);
	newItem->setData(0, Qt::UserRole, parentPath + "/" + folder->getName());

	for (int i = 0; i < folder->size(); ++i) {
		Response_DeckList::Directory *subFolder = dynamic_cast<Response_DeckList::Directory *>(folder->at(i));
		if (subFolder)
			populateDeckList(subFolder, newItem);
		else {
			Response_DeckList::File *file = dynamic_cast<Response_DeckList::File *>(folder->at(i));
			QTreeWidgetItem *newDeck = new QTreeWidgetItem(TWIDeckType);
			newDeck->setIcon(0, fip.icon(QFileIconProvider::File));
			newDeck->setData(0, Qt::DisplayRole, file->getName());
			newDeck->setData(1, Qt::DisplayRole, file->getId());
			newDeck->setTextAlignment(1, Qt::AlignRight);
			newDeck->setData(2, Qt::DisplayRole, file->getUploadTime());

			newItem->addChild(newDeck);
		}
	}
}

void TabDeckStorage::deckListFinished(ProtocolResponse *r)
{
	Response_DeckList *resp = qobject_cast<Response_DeckList *>(r);
	if (!resp)
		return;

	serverDirView->clear();
	populateDeckList(resp->getRoot(), 0);
}

void TabDeckStorage::actUpload()
{
	QModelIndex cur = sortFilter->mapToSource(localDirView->selectionModel()->currentIndex());
	if (localDirModel->isDir(cur))
		return;
	QString filePath = localDirModel->filePath(cur);
	DeckList *deck = new DeckList;
	if (!deck->loadFromFile(filePath, DeckList::CockatriceFormat))
		return;

	QString targetPath;
	QTreeWidgetItem *curRight = serverDirView->currentItem();
	while ((curRight != 0) && (curRight->type() != TWIFolderType))
		curRight = curRight->parent();
	if (curRight)
		targetPath = curRight->data(0, Qt::UserRole).toString();
	qDebug() << "targetPath:" << targetPath;

	Command_DeckUpload *command = new Command_DeckUpload(-1, deck, targetPath);
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(uploadFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void TabDeckStorage::uploadFinished(ProtocolResponse *r)
{
	qDebug() << "buh";
}

void TabDeckStorage::actDownload()
{
}

void TabDeckStorage::actNewFolder()
{
}

void TabDeckStorage::actDelete()
{
}

