#include "setsmodel.h"

SetsModel::SetsModel(CardDatabase *_db, QObject *parent)
	: QAbstractTableModel(parent), sets(_db->getSetList())
{
	sets.sortByKey();
}

SetsModel::~SetsModel()
{
}

int SetsModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return sets.size();
}

QVariant SetsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || (index.column() >= 2) || (index.row() >= rowCount()) || (role != Qt::DisplayRole))
		return QVariant();

	CardSet *set = sets[index.row()];
	switch (index.column()) {
		case 0: return set->getShortName();
		case 1: return set->getLongName();
		default: return QVariant();
	}
}

QVariant SetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
		return QVariant();
	switch (section) {
		case 0: return tr("Short name");
		case 1: return tr("Long name");
		default: return QVariant();
	}
}

Qt::ItemFlags SetsModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags result = QAbstractTableModel::flags(index);
	return result | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions SetsModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QMimeData *SetsModel::mimeData(const QModelIndexList &indexes) const
{
	if (indexes.isEmpty())
		return 0;

	SetsMimeData *result = new SetsMimeData(indexes[0].row());
	return qobject_cast<QMimeData *>(result);
}

bool SetsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int /*column*/, const QModelIndex &parent)
{
	if (action != Qt::MoveAction)
		return false;
	if (row == -1) {
		if (!parent.isValid())
			return false;
		row = parent.row();
	}
	int oldRow = qobject_cast<const SetsMimeData *>(data)->getOldRow();
	beginRemoveRows(QModelIndex(), oldRow, oldRow);
	CardSet *temp = sets.takeAt(oldRow);
	endRemoveRows();
	if (oldRow < row)
		row--;
	beginInsertRows(QModelIndex(), row, row);
	sets.insert(row, temp);
	endInsertRows();

	for (int i = 0; i < sets.size(); i++)
		sets[i]->setSortKey(i);

	return true;
}

QStringList SetsModel::mimeTypes() const
{
	return QStringList() << "application/x-cockatricecardset";
}
