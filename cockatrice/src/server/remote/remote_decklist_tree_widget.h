#ifndef REMOTEDECKLIST_TREEWIDGET_H
#define REMOTEDECKLIST_TREEWIDGET_H

#include <QAbstractItemModel>
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
        DirectoryNode *getParent() const
        {
            return parent;
        }
        QString getName() const
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
        QString getPath() const;
        DirectoryNode *getNodeByPath(QStringList path);
        FileNode *getNodeById(int id) const;
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
        int getId() const
        {
            return id;
        }
        QDateTime getUploadTime() const
        {
            return uploadTime;
        }
    };

    template <typename T> T getNode(const QModelIndex &index) const
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
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    DirectoryNode *getRoot() const
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
    RemoteDeckList_TreeModel::Node *getNode(const QModelIndex &ind) const;
    RemoteDeckList_TreeModel::Node *getCurrentItem() const;
    QList<RemoteDeckList_TreeModel::Node *> getCurrentSelection() const;
    RemoteDeckList_TreeModel::DirectoryNode *getNodeByPath(const QString &path) const;
    RemoteDeckList_TreeModel::FileNode *getNodeById(int id) const;
    void addFileToTree(const ServerInfo_DeckStorage_TreeItem &file, RemoteDeckList_TreeModel::DirectoryNode *parent);
    void addFolderToTree(const ServerInfo_DeckStorage_TreeItem &folder,
                         RemoteDeckList_TreeModel::DirectoryNode *parent);
    void addFolderToTree(const QString &name, RemoteDeckList_TreeModel::DirectoryNode *parent);
    void removeNode(RemoteDeckList_TreeModel::Node *node);
    void refreshTree();
    void clearTree();
};

#endif
