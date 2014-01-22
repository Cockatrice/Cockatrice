#include <QFont>
#include "filterlistmodel.h"
#include "filterlist.h"
#include "cardfilter.h"

FilterListModel::FilterListModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	fList = new FilterList;
	connect(fList,
			SIGNAL(preInsertRow(const FilterListNode *, int)),
			this, SLOT(proxyBeginInsertRow(const FilterListNode *, int)));
	connect(fList,
			SIGNAL(postInsertRow(const FilterListNode *, int)),
			this, SLOT(proxyEndInsertRow(const FilterListNode *, int)));
	connect(fList,
			SIGNAL(preRemoveRow(const FilterListNode *, int)),
			this, SLOT(proxyBeginRemoveRow(const FilterListNode *, int)));
	connect(fList,
			SIGNAL(postRemoveRow(const FilterListNode *, int)),
			this, SLOT(proxyEndRemoveRow(const FilterListNode *, int)));
}

FilterListModel::~FilterListModel()
{
	delete fList;
}

void FilterListModel::proxyBeginInsertRow(const FilterListNode *node, int i) 
{
	int idx;

	idx = node->index();
	if(idx >= 0)
		beginInsertRows(createIndex(idx, 0, (void *) node), i, i);
}

void FilterListModel::proxyEndInsertRow(const FilterListNode *node, int)
{
	int idx;

	idx = node->index();
	if(idx >= 0)
		endInsertRows();
}

void FilterListModel::proxyBeginRemoveRow(const FilterListNode *node, int i) 
{
	int idx;

	idx = node->index();
	if(idx >= 0)
		beginRemoveRows(createIndex(idx, 0, (void *) node), i, i);
}

void FilterListModel::proxyEndRemoveRow(const FilterListNode *node, int)
{
	int idx;

	idx = node->index();
	if(idx >= 0)
		endRemoveRows();
}

FilterListNode *FilterListModel::indexToNode(const QModelIndex &idx) const
{
	void *ip;
	FilterListNode *node;

	if(!idx.isValid())
		return fList;

	ip = idx.internalPointer();
	if(ip == NULL)
		return fList;

	node = static_cast<FilterListNode *>(ip);
	return node;
}

void FilterListModel::addFilter(const CardFilter *f)
{
	emit layoutAboutToBeChanged();
	fList->termNode(f);
	emit layoutChanged();
}

int FilterListModel::rowCount(const QModelIndex &parent) const
{
	const FilterListNode *node;
	int result;

	if(parent.column() > 0)
		return 0;

	node = indexToNode(parent);
	if(node)
		result = node->childCount();
	else
		result = 0;

	return result;
}

int FilterListModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 1;
}

QVariant FilterListModel::data(const QModelIndex &index, int role) const
{
	const FilterListNode *node;

	if (!index.isValid())
		return QVariant();
	if (index.column() >= columnCount())
		return QVariant();

	node = indexToNode(index);
	if(node == NULL)
		return QVariant();

	switch (role) {
		case Qt::FontRole:
			if(!node->isLeaf()) {
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
			if(node->isEnabled())
				return Qt::Checked;
			else
				return Qt::Unchecked;
		default:
			return QVariant();
	}

	return QVariant();
}

bool FilterListModel::setData(const QModelIndex &index,
								const QVariant &value, int role)
{
	FilterListNode *node;

	if (!index.isValid())
		return false;
	if (index.column() >= columnCount())
		return false;
	if (role != Qt::CheckStateRole )
		return false;

	node = indexToNode(index);
	if(node == NULL || node == fList)
		return false;

	Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
	if(state == Qt::Checked)
		node->enable();
    else
		node->disable();

	emit dataChanged(index, index);
    return true;
}	

Qt::ItemFlags FilterListModel::flags(const QModelIndex &index) const
{
	const FilterListNode *node;
	Qt::ItemFlags result;

	if (!index.isValid())
		return 0;

	node = indexToNode(index);
	if(node == NULL)
		return 0;

	result = Qt::ItemIsEnabled;
	if(node == fList)
		return result;

	result |= Qt::ItemIsSelectable;
	result |= Qt::ItemIsUserCheckable;

	return result;
}

QModelIndex FilterListModel::nodeIndex(const FilterListNode *node, int row, int column) const
{
	FilterListNode *child;

	if (column > 0 || row >= node->childCount())
		return QModelIndex();

	child = node->nodeAt(row);
	return createIndex(row, column, child);
}

QModelIndex FilterListModel::index(int row, int column, 
					const QModelIndex &parent) const
{
	const FilterListNode *node;

	if (!hasIndex(row, column, parent))
		return QModelIndex();

	node = indexToNode(parent);
	if(node == NULL)
		return QModelIndex();

	return nodeIndex(node, row, column);
}

QModelIndex FilterListModel::parent(const QModelIndex &ind) const
{
	const FilterListNode *node;
	FilterListNode *parent;
	int row;
	QModelIndex idx;

	if (!ind.isValid())
		return QModelIndex();

	node = indexToNode(ind);
	if(node == NULL || node == fList)
		return QModelIndex();

	parent = node->parent();
	if(parent) {
		row = parent->index();
		if(row < 0)
			return QModelIndex();
		idx = createIndex(row, 0, parent);
		return idx;
	}

	return QModelIndex();
}

bool FilterListModel::removeRows(int row, int count, const QModelIndex & parent)
{
	FilterListNode *node;
	int i, last;

	last = row+count-1;
	if (!parent.isValid() || count < 1 || row < 0)
		return false;

	node = indexToNode(parent);
	if(node == NULL || last >= node->childCount())
		return false;

	printf("delete children in %s\n", node->textCStr());
	fflush(stdout);
	for(i = 0; i < count; i++) {
		printf("  delete %d\n", i);
		fflush(stdout);
		node->deleteAt(row);
	}

	if(node != fList && node->childCount() < 1)
		return removeRow(parent.row(), parent.parent());

	return true;
}
