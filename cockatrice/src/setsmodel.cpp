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
    if (!index.isValid() || (index.column() >= NUM_COLS) || (index.row() >= rowCount()) || (role != Qt::DisplayRole))
        return QVariant();

    CardSet *set = sets[index.row()];
    switch (index.column()) {
        case SortKeyCol: return QString("%1").arg(set->getSortKey(), 4, 10, QChar('0'));
        case SetTypeCol: return set->getSetType();
        case ShortNameCol: return set->getShortName();
        case LongNameCol: return set->getLongName();
        case ReleaseDateCol: return set->getReleaseDate().toString(Qt::ISODate);
        default: return QVariant();
    }
}

QVariant SetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
        return QVariant();
    switch (section) {
        case SortKeyCol: return tr("Key");
        case SetTypeCol: return tr("Set type");
        case ShortNameCol: return tr("Set code");
        case LongNameCol: return tr("Long name");
        case ReleaseDateCol: return tr("Release date");
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
    if (oldRow < row)
        row--;

    swapRows(oldRow, row);

    return true;
}

void SetsModel::swapRows(int oldRow, int newRow)
{
    beginRemoveRows(QModelIndex(), oldRow, oldRow);
    CardSet *temp = sets.takeAt(oldRow);
    endRemoveRows();

    beginInsertRows(QModelIndex(), newRow, newRow);
    sets.insert(newRow, temp);
    endInsertRows();

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void SetsModel::sort(int column, Qt::SortOrder order)
{
    QMap<QString, CardSet *> setMap;
    int numRows = rowCount();
    int row;

    for(row = 0; row < numRows; ++row)
        setMap.insertMulti(index(row, column).data().toString(), sets.at(row));
    
    QList<CardSet *> tmp = setMap.values();
    sets.clear();
    if(order == Qt::AscendingOrder)
    {
        for(row = 0; row < tmp.size(); row++) {
            sets.append(tmp.at(row));
        }
    } else {
        for(row = tmp.size() - 1; row >= 0; row--) {
            sets.append(tmp.at(row));
        }
    }

    emit dataChanged(index(0, 0), index(numRows - 1, columnCount() - 1));
}

void SetsModel::save()
{
    for (int i = 0; i < sets.size(); i++)
        sets[i]->setSortKey(i);

    sets.sortByKey();
}

void SetsModel::restore(CardDatabase *db)
{
    sets = db->getSetList();
    sets.sortByKey();

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

QStringList SetsModel::mimeTypes() const
{
    return QStringList() << "application/x-cockatricecardset";
}
