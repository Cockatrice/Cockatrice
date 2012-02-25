#ifndef TAB_DECK_STORAGE_H
#define TAB_DECK_STORAGE_H

#include "tab.h"
#include "pb/response.pb.h"

class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class RemoteDeckList_TreeWidget;

class TabDeckStorage : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	QTreeView *localDirView;
	QFileSystemModel *localDirModel;
	QSortFilterProxyModel *sortFilter;
	QToolBar *leftToolBar, *rightToolBar;
	RemoteDeckList_TreeWidget *serverDirView;
	QGroupBox *leftGroupBox, *rightGroupBox;
	
	QAction *aOpenLocalDeck, *aUpload, *aOpenRemoteDeck, *aDownload, *aNewFolder, *aDelete;
private slots:
	void actOpenLocalDeck();
	
	void actUpload();
	void uploadFinished(const Response &r);

	void actOpenRemoteDeck();
	void openRemoteDeckFinished(const Response &r);
	
	void actDownload();
	void downloadFinished(const Response &r);

	void actNewFolder();
	void newFolderFinished(Response::ResponseCode resp);

	void actDelete();
	void deleteFolderFinished(Response::ResponseCode resp);
	void deleteDeckFinished(Response::ResponseCode resp);
public:
	TabDeckStorage(TabSupervisor *_tabSupervisor, AbstractClient *_client);
	void retranslateUi();
	QString getTabText() const { return tr("Deck storage"); }
};

#endif
