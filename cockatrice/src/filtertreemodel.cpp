#include "filtertreemodel.h"
#include "cardfilter.h"
#include "filtertree.h"
#include <QFont>

FilterTreeModel::FilterTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    fTree = new FilterTree;
    connect(fTree, SIGNAL(preInsertRow(const FilterTreeNode *, int)), this,
            SLOT(proxyBeginInsertRow(const FilterTreeNode *, int)));
    connect(fTree, SIGNAL(postInsertRow(const FilterTreeNode *, int)), this,
            SLOT(proxyEndInsertRow(const FilterTreeNode *, int)));
    connect(fTree, SIGNAL(preRemoveRow(const FilterTreeNode *, int)), this,
            SLOT(proxyBeginRemoveRow(const FilterTreeNode *, int)));
    connect(fTree, SIGNAL(postRemoveRow(const FilterTreeNode *, int)), this,
            SLOT(proxyEndRemoveRow(const FilterTreeNode *, int)));
}

FilterTreeModel::~FilterTreeModel()
{
    delete fTree;
}

void FilterTreeModel::proxyBeginInsertRow(const FilterTreeNode *node, int i)
{
    int idx;

    idx = node->index();
    if (idx >= 0)
        beginInsertRows(createIndex(idx, 0, (void *)node), i, i);
}

void FilterTreeModel::proxyEndInsertRow(const FilterTreeNode *node, int)
{
    int idx;

    idx = node->index();
    if (idx >= 0)
        endInsertRows();
}

void FilterTreeModel::proxyBeginRemoveRow(const FilterTreeNode *node, int i)
{
    int idx;

    idx = node->index();
    if (idx >= 0)
        beginRemoveRows(createIndex(idx, 0, (void *)node), i, i);
}

void FilterTreeModel::proxyEndRemoveRow(const FilterTreeNode *node, int)
{
    int idx;

    idx = node->index();
    if (idx >= 0)
        endRemoveRows();
}

FilterTreeNode *FilterTreeModel::indexToNode(const QModelIndex &idx) const
{
    void *ip;
    FilterTreeNode *node;

    if (!idx.isValid())
        return fTree;

    ip = idx.internalPointer();
    if (ip == NULL)
        return fTree;

    node = static_cast<FilterTreeNode *>(ip);
    return node;
}

void FilterTreeModel::addFilter(const CardFilter *f)
{
    emit layoutAboutToBeChanged();
    fTree->termNode(f);
    emit layoutChanged();
}

int FilterTreeModel::rowCount(const QModelIndex &parent) const
{
    const FilterTreeNode *node;
    int result;

    if (parent.column() > 0)
        return 0;

    node = indexToNode(parent);
    if (node)
        result = node->childCount();
    else
        result = 0;

    return result;
}

int FilterTreeModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant FilterTreeModel::data(const QModelIndex &index, int role) const
{
    const FilterTreeNode *node;

    if (!index.isValid())
        return QVariant();
    if (index.column() >= columnCount())
        return QVariant();

    node = indexToNode(index);
    if (node == NULL)
        return QVariant();

    switch (role) {
        case Qt::FontRole:
            if (!node->isLeaf()) {
                QFont f;
                f.setBold(true);
                return f;
            }
            break;
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return node->text();
        case Qt::CheckStateRole:
            if (node->isEnabled())
                return Qt::Checked;
            else
                return Qt::Unchecked;
        default:
            return QVariant();
    }

    return QVariant();
}

bool FilterTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    FilterTreeNode *node;

    if (!index.isValid())
        return false;
    if (index.column() >= columnCount())
        return false;
    if (role != Qt::CheckStateRole)
        return false;

    node = indexToNode(index);
    if (node == NULL || node == fTree)
        return false;

    Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
    if (state == Qt::Checked)
        node->enable();
    else
        node->disable();

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags FilterTreeModel::flags(const QModelIndex &index) const
{
    const FilterTreeNode *node;
    Qt::ItemFlags result;

    if (!index.isValid())
        return 0;

    node = indexToNode(index);
    if (node == NULL)
        return 0;

    result = Qt::ItemIsEnabled;
    if (node == fTree)
        return result;

    result |= Qt::ItemIsSelectable;
    result |= Qt::ItemIsUserCheckable;

    return result;
}

QModelIndex FilterTreeModel::nodeIndex(const FilterTreeNode *node, int row, int column) const
{
    FilterTreeNode *child;

    if (column > 0 || row >= node->childCount())
        return QModelIndex();

    child = node->nodeAt(row);
    return createIndex(row, column, child);
}

QModelIndex FilterTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    const FilterTreeNode *node;

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    node = indexToNode(parent);
    if (node == NULL)
        return QModelIndex();

    return nodeIndex(node, row, column);
}

QModelIndex FilterTreeModel::parent(const QModelIndex &ind) const
{
    const FilterTreeNode *node;
    FilterTreeNode *parent;
    QModelIndex idx;

    if (!ind.isValid())
        return QModelIndex();

    node = indexToNode(ind);
    if (node == NULL || node == fTree)
        return QModelIndex();

    parent = node->parent();
    if (parent) {
        int row = parent->index();
        if (row < 0)
            return QModelIndex();
        idx = createIndex(row, 0, parent);
        return idx;
    }

    return QModelIndex();
}

bool FilterTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    FilterTreeNode *node;
    int i, last;

    last = row + count - 1;
    if (!parent.isValid() || count < 1 || row < 0)
        return false;

    node = indexToNode(parent);
    if (node == NULL || last >= node->childCount())
        return false;

    for (i = 0; i < count; i++)
        node->deleteAt(row);

    if (node != fTree && node->childCount() < 1)
        return removeRow(parent.row(), parent.parent());

    return true;
}
