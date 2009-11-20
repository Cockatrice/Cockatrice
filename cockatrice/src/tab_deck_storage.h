#ifndef TAB_DECK_STORAGE_H
#define TAB_DECK_STORAGE_H

#include <QWidget>
#include "protocol_datastructures.h"

class Client;
class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class ProtocolResponse;

class TabDeckStorage : public QWidget {
	Q_OBJECT
private:
	Client *client;
	QTreeView *localDirView;
	QFileSystemModel *localDirModel;
	QSortFilterProxyModel *sortFilter;
	QToolBar *leftToolBar, *rightToolBar;
	QTreeWidget *serverDirView;
	QGroupBox *leftGroupBox, *rightGroupBox;
	
	QAction *aUpload, *aDownload, *aNewFolder, *aDelete;
	void addFileToTree(DeckList_File *file, QTreeWidgetItem *parent);
	void populateDeckList(DeckList_Directory *folder, QTreeWidgetItem *parent);
	void refreshServerList();
private slots:
	void deckListFinished(ProtocolResponse *r);

	void actUpload();
	void uploadFinished(ProtocolResponse *r);

	void actDownload();
	void downloadFinished(ProtocolResponse *r);

	void actNewFolder();
	void newFolderFinished(ResponseCode resp);

	void actDelete();
	void deleteFinished(ResponseCode resp);
public:
	TabDeckStorage(Client *_client);
	void retranslateUi();
};

#endif
