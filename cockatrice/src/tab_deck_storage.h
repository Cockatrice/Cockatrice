#ifndef TAB_DECK_STORAGE_H
#define TAB_DECK_STORAGE_H

#include <QWidget>
#include "protocol.h"

class Client;
class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;

class TabDeckStorage : public QWidget {
	Q_OBJECT
private:
	Client *client;
	QTreeView *localDirView;
	QFileSystemModel *localDirModel;
	QSortFilterProxyModel *sortFilter;
	QToolBar *toolBar;
	QTreeWidget *serverDirView;
	QGroupBox *leftGroupBox, *rightGroupBox;
	
	QAction *aUpload, *aDownload, *aNewFolder, *aDelete;
	void populateDeckList(Response_DeckList::Directory *folder, QTreeWidgetItem *parent);
	void refreshServerList();
private slots:
	void deckListFinished(ProtocolResponse *r);

	void actUpload();
	void uploadFinished(ProtocolResponse *r);

	void actDownload();
	void actNewFolder();
	void actDelete();
public:
	TabDeckStorage(Client *_client);
	void retranslateUi();
};

#endif
