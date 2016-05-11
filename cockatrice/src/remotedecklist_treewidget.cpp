#include <QFileIconProvider>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "remotedecklist_treewidget.h"
#include "abstractclient.h"

#include "pending_command.h"
#include "pb/command_deck_list.pb.h"
#include "pb/response_deck_list.pb.h"
#include "pb/serverinfo_deckstorage.pb.h"

RemoteDeckList_TreeModel::DirectoryNode::DirectoryNode(const QString &_name, RemoteDeckList_TreeModel::DirectoryNode *_parent)
    : RemoteDeckList_TreeModel::Node(_name, _parent)
{
}

RemoteDeckList_TreeModel::DirectoryNode::~DirectoryNode()
{
    clearTree();
}

void RemoteDeckList_TreeModel::DirectoryNode::clearTree()
{
    for (int i = 0; i < size(); ++i)
        delete at(i);
    clear();
}

QString RemoteDeckList_TreeModel::DirectoryNode::getPath() const
{
    if (parent) {
        QString parentPath = parent->getPath();
        if (parentPath.isEmpty())
            return name;
        else
            return parentPath + "/" + name;
    } else
        return name;
}

RemoteDeckList_TreeModel::DirectoryNode *RemoteDeckList_TreeModel::DirectoryNode::getNodeByPath(QStringList path)
{
    QString pathItem;
    if (parent) {
        if (path.isEmpty())
            return this;
        pathItem = path.takeFirst();
        if (pathItem.isEmpty() && name.isEmpty())
            return this;
    }
    
    for (int i = 0; i < size(); ++i) {
        DirectoryNode *node = dynamic_cast<DirectoryNode *>(at(i));
        if (!node)
            continue;
        if (node->getName() == pathItem)
            return node->getNodeByPath(path);
    }
    return 0;
}

RemoteDeckList_TreeModel::FileNode *RemoteDeckList_TreeModel::DirectoryNode::getNodeById(int id) const
{
    for (int i = 0; i < size(); ++i) {
        DirectoryNode *node = dynamic_cast<DirectoryNode *>(at(i));
        if (node) {
            FileNode *result = node->getNodeById(id);
            if (result)
                return result;
        } else {
            FileNode *file = dynamic_cast<FileNode *>(at(i));
            if (file->getId() == id)
                return file;
        }
    }
    return 0;
}

RemoteDeckList_TreeModel::RemoteDeckList_TreeModel(AbstractClient *_client, QObject *parent)
    : QAbstractItemModel(parent), client(_client)
{
    QFileIconProvider fip;
    dirIcon = fip.icon(QFileIconProvider::Folder);
    fileIcon = fip.icon(QFileIconProvider::File);

    root = new DirectoryNode;
    refreshTree();
}

RemoteDeckList_TreeModel::~RemoteDeckList_TreeModel()
{
    delete root;
}

int RemoteDeckList_TreeModel::rowCount(const QModelIndex &parent) const
{
    DirectoryNode *node = getNode<DirectoryNode *>(parent);
    if (node)
        return node->size();
    else
        return 0;
}

int RemoteDeckList_TreeModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 3;
}

QVariant RemoteDeckList_TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.column() >= 3)
        return QVariant();

    Node *temp = static_cast<Node *>(index.internalPointer());
    FileNode *file = dynamic_cast<FileNode *>(temp);
    if (!file) {
        DirectoryNode *node = dynamic_cast<DirectoryNode *>(temp);
        switch (role) {
            case Qt::DisplayRole: {
                switch (index.column()) {
                    case 0: return node->getName();
                    default:
                        return QVariant();
                }
            }
            case Qt::DecorationRole:
                return index.column() == 0 ? dirIcon : QVariant();
            default: return QVariant();
        }
    } else {
        switch (role) {
            case Qt::DisplayRole: {
                switch (index.column()) {
                    case 0: return file->getName();
                    case 1: return file->getId();
                    case 2: return file->getUploadTime();
                    default:
                        return QVariant();
                }
            }
            case Qt::DecorationRole:
                return index.column() == 0 ? fileIcon : QVariant();
            case Qt::TextAlignmentRole:
                return index.column() == 1 ? Qt::AlignRight : Qt::AlignLeft;
            default: return QVariant();
        }
    }
}

QVariant RemoteDeckList_TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();
    switch (role) {
        case Qt::TextAlignmentRole:
            return section == 1 ? Qt::AlignRight : Qt::AlignLeft;
        case Qt::DisplayRole: {
            switch (section) {
                case 0: return tr("Name");
                case 1: return tr("ID");
                case 2: return tr("Upload time");
                default: return QVariant();
            }
        }
        default: return QVariant();
    }
}

QModelIndex RemoteDeckList_TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DirectoryNode *parentNode = getNode<DirectoryNode *>(parent);
    if (row >= parentNode->size())
        return QModelIndex();

    return createIndex(row, column, parentNode->at(row));
}

QModelIndex RemoteDeckList_TreeModel::parent(const QModelIndex &ind) const
{
    if (!ind.isValid())
        return QModelIndex();

    return nodeToIndex(static_cast<Node *>(ind.internalPointer())->getParent());
}

Qt::ItemFlags RemoteDeckList_TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex RemoteDeckList_TreeModel::nodeToIndex(Node *node) const
{
    if (node == root)
        return QModelIndex();
    return createIndex(node->getParent()->indexOf(node), 0, node);
}

void RemoteDeckList_TreeModel::addFileToTree(const ServerInfo_DeckStorage_TreeItem &file, DirectoryNode *parent)
{
    const ServerInfo_DeckStorage_File &fileInfo = file.file();
    QDateTime time;
    time.setTime_t(fileInfo.creation_time());
    
    beginInsertRows(nodeToIndex(parent), parent->size(), parent->size());
    parent->append(new FileNode(QString::fromStdString(file.name()), file.id(), time, parent));
    endInsertRows();
}

void RemoteDeckList_TreeModel::addFolderToTree(const ServerInfo_DeckStorage_TreeItem &folder, DirectoryNode *parent)
{
    DirectoryNode *newItem = addNamedFolderToTree(QString::fromStdString(folder.name()), parent);
    const ServerInfo_DeckStorage_Folder &folderInfo = folder.folder();
    const int folderItemsSize = folderInfo.items_size();
    for (int i = 0; i < folderItemsSize; ++i) {
        const ServerInfo_DeckStorage_TreeItem &subItem = folderInfo.items(i);
        if (subItem.has_folder())
            addFolderToTree(subItem, newItem);
        else
            addFileToTree(subItem, newItem);
    }
}

RemoteDeckList_TreeModel::DirectoryNode *RemoteDeckList_TreeModel::addNamedFolderToTree(const QString &name, DirectoryNode *parent)
{
    DirectoryNode *newItem = new DirectoryNode(name, parent);
    beginInsertRows(nodeToIndex(parent), parent->size(), parent->size());
    parent->append(newItem);
    endInsertRows();
    return newItem;
}

void RemoteDeckList_TreeModel::removeNode(RemoteDeckList_TreeModel::Node *node)
{
    int ind = node->getParent()->indexOf(node);
    beginRemoveRows(nodeToIndex(node->getParent()), ind, ind);
    node->getParent()->removeAt(ind);
    endRemoveRows();
    delete node;
}

void RemoteDeckList_TreeModel::refreshTree()
{
    PendingCommand *pend = client->prepareSessionCommand(Command_DeckList());
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(deckListFinished(const Response &)));
    
    client->sendCommand(pend);
}

void RemoteDeckList_TreeModel::deckListFinished(const Response &r)
{
    const Response_DeckList &resp = r.GetExtension(Response_DeckList::ext);

    beginResetModel();

    root->clearTree();

    endResetModel();
    
    ServerInfo_DeckStorage_TreeItem tempRoot;
    tempRoot.set_id(0);
    tempRoot.mutable_folder()->CopyFrom(resp.root());
    addFolderToTree(tempRoot, root);
    
    emit treeRefreshed();
}

RemoteDeckList_TreeWidget::RemoteDeckList_TreeWidget(AbstractClient *_client, QWidget *parent)
    : QTreeView(parent)
{
    treeModel = new RemoteDeckList_TreeModel(_client, this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(treeModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    setModel(proxyModel);
    connect(treeModel, SIGNAL(treeRefreshed()), this, SLOT(expandAll()));

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setUniformRowHeights(true);
    setSortingEnabled(true);
    proxyModel->sort(0, Qt::AscendingOrder);
    header()->setSortIndicator(0, Qt::AscendingOrder);
}

RemoteDeckList_TreeModel::Node *RemoteDeckList_TreeWidget::getNode(const QModelIndex &ind) const
{
    return treeModel->getNode<RemoteDeckList_TreeModel::Node *>(proxyModel->mapToSource(ind));
}

RemoteDeckList_TreeModel::Node *RemoteDeckList_TreeWidget::getCurrentItem() const
{
    return getNode(selectionModel()->currentIndex());
}

RemoteDeckList_TreeModel::DirectoryNode *RemoteDeckList_TreeWidget::getNodeByPath(const QString &path) const
{
    return treeModel->getRoot()->getNodeByPath(path.split("/"));
}

RemoteDeckList_TreeModel::FileNode *RemoteDeckList_TreeWidget::getNodeById(int id) const
{
    return treeModel->getRoot()->getNodeById(id);
}

void RemoteDeckList_TreeWidget::addFileToTree(const ServerInfo_DeckStorage_TreeItem &file, RemoteDeckList_TreeModel::DirectoryNode *parent)
{
    treeModel->addFileToTree(file, parent);
}

void RemoteDeckList_TreeWidget::addFolderToTree(const ServerInfo_DeckStorage_TreeItem &folder, RemoteDeckList_TreeModel::DirectoryNode *parent)
{
    treeModel->addFolderToTree(folder, parent);
}

void RemoteDeckList_TreeWidget::addFolderToTree(const QString &name, RemoteDeckList_TreeModel::DirectoryNode *parent)
{
    treeModel->addNamedFolderToTree(name, parent);
}

void RemoteDeckList_TreeWidget::removeNode(RemoteDeckList_TreeModel::Node *node)
{
    treeModel->removeNode(node);
}

void RemoteDeckList_TreeWidget::refreshTree()
{
    treeModel->refreshTree();
}
