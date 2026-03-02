/**
 * @file tab_deck_storage.h
 * @ingroup DeckStorageWidgets
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_DECK_STORAGE_H
#define TAB_DECK_STORAGE_H

#include "../interface/widgets/server/remote/remote_decklist_tree_widget.h"
#include "tab.h"

#include <libcockatrice/network/client/abstract/abstract_client.h>

struct LoadedDeck;
class ServerInfo_User;
class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class CommandContainer;
class Response;

class TabDeckStorage : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    QTreeView *localDirView;
    QFileSystemModel *localDirModel;
    QToolBar *leftToolBar, *rightToolBar;
    RemoteDeckList_TreeWidget *serverDirView;
    QGroupBox *leftGroupBox, *rightGroupBox;

    QAction *aOpenLocalDeck, *aRenameLocal, *aUpload, *aNewLocalFolder, *aDeleteLocalDeck;
    QAction *aOpenDecksFolder;
    QAction *aOpenRemoteDeck, *aDownload, *aNewFolder, *aDeleteRemoteDeck;
    QString getTargetPath() const;

    void setRemoteEnabled(bool enabled);

    void uploadDeck(const QString &filePath, const QString &targetPath);
    void deleteRemoteDeck(const RemoteDeckList_TreeModel::Node *node);

    void downloadNodeAtIndex(const QModelIndex &curLeft, const QModelIndex &curRight);

private slots:
    void handleConnected(const ServerInfo_User &userInfo);
    void handleConnectionChanged(ClientStatus status);

    void actLocalDoubleClick(const QModelIndex &curLeft);
    void actOpenLocalDeck();

    void actRenameLocal();

    void actUpload();
    void uploadFinished(const Response &r, const CommandContainer &commandContainer);

    void actNewLocalFolder();
    void actDeleteLocalDeck();

    void actOpenDecksFolder();

    void actRemoteDoubleClick(const QModelIndex &curRight);
    void actOpenRemoteDeck();
    void openRemoteDeckFinished(const Response &r, const CommandContainer &commandContainer);

    void actDownload();
    void downloadFinished(const Response &r, const CommandContainer &commandContainer, const QVariant &extraData);

    void actNewFolder();
    void newFolderFinished(const Response &response, const CommandContainer &commandContainer);

    void actDeleteRemoteDeck();
    void deleteFolderFinished(const Response &response, const CommandContainer &commandContainer);
    void deleteDeckFinished(const Response &response, const CommandContainer &commandContainer);

public:
    TabDeckStorage(TabSupervisor *_tabSupervisor, AbstractClient *_client, const ServerInfo_User *currentUserInfo);
    void retranslateUi() override;
    QString getTabText() const override
    {
        return tr("Deck Storage");
    }
signals:
    void openDeckEditor(const LoadedDeck &deck);
};

#endif
