#ifndef TAB_DECK_STORAGE_H
#define TAB_DECK_STORAGE_H

#include "tab.h"
#include "protocol.h"

class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class ProtocolResponse;
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
	void uploadFinished(ProtocolResponse *r);

	void actOpenRemoteDeck();
	void openRemoteDeckFinished(ProtocolResponse *r);
	
	void actDownload();
	void downloadFinished(ProtocolResponse *r);

	void actNewFolder();
	void newFolderFinished(ResponseCode resp);

	void actDelete();
	void deleteFinished(ResponseCode resp);
public:
	TabDeckStorage(AbstractClient *_client);
	void retranslateUi();
	QString getTabText() const { return tr("Deck storage"); }
};

#endif
