#include "filter_tree_model.h"

#include <QFont>
#include <libcockatrice/filters/filter_card.h>
#include <libcockatrice/filters/filter_tree.h>

FilterTreeModel::FilterTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    fTree = new FilterTree;
    connect(fTree, &FilterTree::preInsertRow, this, &FilterTreeModel::proxyBeginInsertRow);
    connect(fTree, &FilterTree::postInsertRow, this, &FilterTreeModel::proxyEndInsertRow);
    connect(fTree, &FilterTree::preRemoveRow, this, &FilterTreeModel::proxyBeginRemoveRow);
    connect(fTree, &FilterTree::postRemoveRow, this, &FilterTreeModel::proxyEndRemoveRow);
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

void FilterTreeModel::removeFilter(const CardFilter *f)
{
    emit layoutAboutToBeChanged();
    fTree->removeFilter(f);
    emit layoutChanged();
}

void FilterTreeModel::clearFiltersOfType(CardFilter::Attr filterType)
{
    emit layoutAboutToBeChanged();

    // Recursively remove all nodes with the given filter type
    fTree->removeFiltersByAttr(filterType);

    emit layoutChanged();
}

QList<const CardFilter *> FilterTreeModel::getFiltersOfType(CardFilter::Attr filterType) const
{
    QList<const CardFilter *> filters;
    if (!fTree) {
        return filters;
    }

    std::function<void(const FilterTreeNode *)> traverse;
    traverse = [&](const FilterTreeNode *node) {
        if (const auto *filterItem = dynamic_cast<const FilterItem *>(node)) {
            if (filterItem->attr() == filterType) {
                QString text = filterItem->text();
                filters.append(new CardFilter(text, filterItem->type(), filterItem->attr()));
            }
        }
        for (int i = 0; i < node->childCount(); ++i) {
            traverse(node->nodeAt(i));
        }
    };

    traverse(fTree);
    return filters;
}

QList<const CardFilter *> FilterTreeModel::allFilters() const
{
    QList<const CardFilter *> filters;
    if (!fTree) {
        return filters;
    }

    std::function<void(const FilterTreeNode *)> traverse;
    traverse = [&](const FilterTreeNode *node) {
        if (const auto *filterItem = dynamic_cast<const FilterItem *>(node)) {
            QString text = filterItem->text();
            filters.append(new CardFilter(text, filterItem->type(), filterItem->attr()));
        }
        for (int i = 0; i < node->childCount(); ++i) {
            traverse(node->nodeAt(i));
        }
    };

    traverse(fTree);
    return filters;
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
        return Qt::NoItemFlags;

    node = indexToNode(index);
    if (node == NULL)
        return Qt::NoItemFlags;

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

void FilterTreeModel::clear()
{
    emit layoutAboutToBeChanged();
    fTree->clear();
    emit layoutChanged();
}
