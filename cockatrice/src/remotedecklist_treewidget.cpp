#include <QFileIconProvider>
#include <QHeaderView>
#include "remotedecklist_treewidget.h"
#include "protocol_items.h"
#include "client.h"

RemoteDeckList_TreeWidget::RemoteDeckList_TreeWidget(Client *_client, QWidget *parent)
	: QTreeWidget(parent), client(_client)
{
	header()->setResizeMode(QHeaderView::ResizeToContents);
	setColumnCount(3);

	refreshTree();
	retranslateUi();
}

void RemoteDeckList_TreeWidget::retranslateUi()
{
	headerItem()->setText(0, tr("Name"));
	headerItem()->setText(1, tr("ID"));
	headerItem()->setText(2, tr("Upload time"));
	headerItem()->setTextAlignment(1, Qt::AlignRight);
}

void RemoteDeckList_TreeWidget::addFileToTree(DeckList_File *file, QTreeWidgetItem *parent)
{
	QFileIconProvider fip;
	QTreeWidgetItem *newDeck = new QTreeWidgetItem(TWIDeckType);
	newDeck->setIcon(0, fip.icon(QFileIconProvider::File));
	newDeck->setData(0, Qt::DisplayRole, file->getName());
	newDeck->setData(1, Qt::DisplayRole, file->getId());
	newDeck->setTextAlignment(1, Qt::AlignRight);
	newDeck->setData(2, Qt::DisplayRole, file->getUploadTime());

	parent->addChild(newDeck);
	sortItems(0, Qt::AscendingOrder);
}

void RemoteDeckList_TreeWidget::addFolderToTree(DeckList_Directory *folder, QTreeWidgetItem *parent)
{
	QFileIconProvider fip;
	QTreeWidgetItem *newItem = new QTreeWidgetItem(TWIFolderType);
	newItem->setIcon(0, fip.icon(QFileIconProvider::Folder));
	newItem->setText(0, parent ? folder->getName() : "/");
	if (parent) {
		parent->addChild(newItem);

		QString path = parent->data(0, Qt::UserRole).toString();
		if (path.isEmpty())
			newItem->setData(0, Qt::UserRole, folder->getName());
		else
			newItem->setData(0, Qt::UserRole, path + "/" + folder->getName());
	} else {
		addTopLevelItem(newItem);
		newItem->setData(0, Qt::UserRole, QString());
	}

	const QList<DeckList_TreeItem *> &folderItems = folder->getTreeItems();
	for (int i = 0; i < folderItems.size(); ++i) {
		DeckList_Directory *subFolder = dynamic_cast<DeckList_Directory *>(folderItems[i]);
		if (subFolder)
			addFolderToTree(subFolder, newItem);
		else
			addFileToTree(dynamic_cast<DeckList_File *>(folderItems[i]), newItem);
	}
	sortItems(0, Qt::AscendingOrder);
}

void RemoteDeckList_TreeWidget::refreshTree()
{
	Command_DeckList *command = new Command_DeckList;
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(deckListFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void RemoteDeckList_TreeWidget::deckListFinished(ProtocolResponse *r)
{
	Response_DeckList *resp = qobject_cast<Response_DeckList *>(r);
	if (!resp)
		return;

	clear();
	addFolderToTree(resp->getRoot(), 0);
	expandAll();
}
