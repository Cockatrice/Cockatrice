#ifndef TAB_DECK_STORAGE_H
#define TAB_DECK_STORAGE_H

#include "tab.h"

class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class RemoteDeckList_TreeWidget;
class CommandContainer;
class Response;

class TabDeckStorage : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	QTreeView *localDirView;
	QFileSystemModel *localDirModel;
	QToolBar *leftToolBar, *rightToolBar;
	RemoteDeckList_TreeWidget *serverDirView;
	QGroupBox *leftGroupBox, *rightGroupBox;
	
	QAction *aOpenLocalDeck, *aUpload, *aOpenRemoteDeck, *aDownload, *aNewFolder, *aDelete;
private slots:
	void actOpenLocalDeck();
	
	void actUpload();
	void uploadFinished(const Response &r, const CommandContainer &commandContainer);

	void actOpenRemoteDeck();
	void openRemoteDeckFinished(const Response &r);
	
	void actDownload();
	void downloadFinished(const Response &r, const CommandContainer &commandContainer, const QVariant &extraData);

	void actNewFolder();
	void newFolderFinished(const Response &response, const CommandContainer &commandContainer);

	void actDelete();
	void deleteFolderFinished(const Response &response, const CommandContainer &commandContainer);
	void deleteDeckFinished(const Response &response, const CommandContainer &commandContainer);
public:
	TabDeckStorage(TabSupervisor *_tabSupervisor, AbstractClient *_client);
	void retranslateUi();
	QString getTabText() const { return tr("Deck storage"); }
};

#endif
