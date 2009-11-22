#ifndef REMOTEDECKLIST_TREEWIDGET_H
#define REMOTEDECKLIST_TREEWIDGET_H

#include <QTreeWidget>

class ProtocolResponse;
class Client;
class DeckList_File;
class DeckList_Directory;

enum { TWIFolderType = QTreeWidgetItem::UserType + 1, TWIDeckType = QTreeWidgetItem::UserType + 2 };

class RemoteDeckList_TreeWidget : public QTreeWidget {
	Q_OBJECT
private:
	Client *client;
private slots:
	void deckListFinished(ProtocolResponse *r);
public:
	RemoteDeckList_TreeWidget(Client *_client, QWidget *parent = 0);
	void retranslateUi();
	void addFileToTree(DeckList_File *file, QTreeWidgetItem *parent);
	void addFolderToTree(DeckList_Directory *folder, QTreeWidgetItem *parent);
	void refreshTree();
};

#endif
