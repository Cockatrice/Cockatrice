/**
 * @file remote_decklist_tree_widget.h
 * @ingroup NetworkingWidgets
 * @ingroup DeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef REMOTEDECKLIST_TREEWIDGET_H
#define REMOTEDECKLIST_TREEWIDGET_H

#include <QDateTime>
#include <QTreeView>

class Response;
class AbstractClient;
class QSortFilterProxyModel;
class ServerInfo_DeckStorage_TreeItem;

class RemoteDeckList_TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    class DirectoryNode;
    class FileNode;
    class Node
    {
    protected:
        DirectoryNode *parent;
        QString name;

    public:
        explicit Node(const QString &_name, DirectoryNode *_parent = nullptr) : parent(_parent), name(_name)
        {
        }
        virtual ~Node() = default;
        [[nodiscard]] DirectoryNode *getParent() const
        {
            return parent;
        }
        [[nodiscard]] QString getName() const
        {
            return name;
        }
    };
    class DirectoryNode : public Node, public QList<Node *>
    {
    public:
        explicit DirectoryNode(const QString &_name = QString(), DirectoryNode *_parent = nullptr);
        ~DirectoryNode() override;
        void clearTree();
        [[nodiscard]] QString getPath() const;
        DirectoryNode *getNodeByPath(QStringList path);
        [[nodiscard]] FileNode *getNodeById(int id) const;
    };
    class FileNode : public Node
    {
    private:
        int id;
        QDateTime uploadTime;

    public:
        FileNode(const QString &_name, int _id, const QDateTime &_uploadTime, DirectoryNode *_parent = nullptr)
            : Node(_name, _parent), id(_id), uploadTime(_uploadTime)
        {
        }
        [[nodiscard]] int getId() const
        {
            return id;
        }
        [[nodiscard]] QDateTime getUploadTime() const
        {
            return uploadTime;
        }
    };

    template <typename T> [[nodiscard]] T getNode(const QModelIndex &index) const
    {
        if (!index.isValid())
            return dynamic_cast<T>(root);
        return dynamic_cast<T>(static_cast<Node *>(index.internalPointer()));
    }

private:
    AbstractClient *client;
    DirectoryNode *root;

    QIcon fileIcon, dirIcon;

    QModelIndex nodeToIndex(Node *node) const;
signals:
    void treeRefreshed();
private slots:
    void deckListFinished(const Response &r);

public:
    explicit RemoteDeckList_TreeModel(AbstractClient *_client, QObject *parent = nullptr);
    ~RemoteDeckList_TreeModel() override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    [[nodiscard]] DirectoryNode *getRoot() const
    {
        return root;
    }
    void addFileToTree(const ServerInfo_DeckStorage_TreeItem &file, DirectoryNode *parent);
    void addFolderToTree(const ServerInfo_DeckStorage_TreeItem &folder, DirectoryNode *parent);
    DirectoryNode *addNamedFolderToTree(const QString &name, DirectoryNode *parent);
    void removeNode(Node *node);
    void refreshTree();
    void clearTree();
};

class RemoteDeckList_TreeWidget : public QTreeView
{
private:
    RemoteDeckList_TreeModel *treeModel;
    QSortFilterProxyModel *proxyModel;

public:
    explicit RemoteDeckList_TreeWidget(AbstractClient *_client, QWidget *parent = nullptr);
    [[nodiscard]] RemoteDeckList_TreeModel::Node *getNode(const QModelIndex &ind) const;
    [[nodiscard]] RemoteDeckList_TreeModel::Node *getCurrentItem() const;
    [[nodiscard]] QList<RemoteDeckList_TreeModel::Node *> getCurrentSelection() const;
    [[nodiscard]] RemoteDeckList_TreeModel::DirectoryNode *getNodeByPath(const QString &path) const;
    [[nodiscard]] RemoteDeckList_TreeModel::FileNode *getNodeById(int id) const;
    void addFileToTree(const ServerInfo_DeckStorage_TreeItem &file, RemoteDeckList_TreeModel::DirectoryNode *parent);
    void addFolderToTree(const ServerInfo_DeckStorage_TreeItem &folder,
                         RemoteDeckList_TreeModel::DirectoryNode *parent);
    void addFolderToTree(const QString &name, RemoteDeckList_TreeModel::DirectoryNode *parent);
    void removeNode(RemoteDeckList_TreeModel::Node *node);
    void refreshTree();
    void clearTree();
};

#endif
