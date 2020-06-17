#include "setsmodel.h"

SetsModel::SetsModel(CardDatabase *_db, QObject *parent) : QAbstractTableModel(parent), sets(_db->getSetList())
{
    sets.sortByKey();
    for (const CardSetPtr &set : sets) {
        if (set->getEnabled())
            enabledSets.insert(set);
    }
}

SetsModel::~SetsModel() = default;

int SetsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return sets.size();
}

QVariant SetsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.column() >= NUM_COLS) || (index.row() >= rowCount()))
        return QVariant();

    CardSetPtr set = sets[index.row()];

    if (index.column() == EnabledCol) {
        switch (role) {
            case SortRole:
                return enabledSets.contains(set) ? "1" : "0";
            case Qt::CheckStateRole:
                return static_cast<int>(enabledSets.contains(set) ? Qt::Checked : Qt::Unchecked);
            case Qt::DisplayRole:
            default:
                return QVariant();
        }
    }

    if (role != Qt::DisplayRole && role != SortRole)
        return QVariant();

    switch (index.column()) {
        case SortKeyCol:
            return QString("%1").arg(set->getSortKey(), 8, 10, QChar('0'));
        case IsKnownCol:
            return set->getIsKnown();
        case SetTypeCol:
            return set->getSetType();
        case ShortNameCol:
            return set->getShortName();
        case LongNameCol:
            return set->getLongName();
        case ReleaseDateCol:
            return set->getReleaseDate().toString(Qt::ISODate);
        default:
            return QVariant();
    }
}

bool SetsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == EnabledCol) {
        toggleRow(index.row(), value == Qt::Checked);
        return true;
    }
    return false;
}

QVariant SetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
        return QVariant();
    switch (section) {
        case SortKeyCol:
            return QString("Key"); /* no tr() for translations needed, column just used for sorting --> hidden */
        case IsKnownCol:
            return QString(
                "Is known"); /* no tr() for translations needed, column is just used for sorting --> hidden */
        case EnabledCol:
            return tr("Enabled");
        case SetTypeCol:
            return tr("Set type");
        case ShortNameCol:
            return tr("Set code");
        case LongNameCol:
            return tr("Long name");
        case ReleaseDateCol:
            return tr("Release date");
        default:
            return QVariant();
    }
}

Qt::ItemFlags SetsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    if (index.column() == EnabledCol)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
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

bool SetsModel::dropMimeData(const QMimeData *data,
                             Qt::DropAction action,
                             int row,
                             int /*column*/,
                             const QModelIndex &parent)
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

void SetsModel::toggleRow(int row, bool enable)
{
    CardSetPtr temp = sets.at(row);

    if (enable)
        enabledSets.insert(temp);
    else
        enabledSets.remove(temp);

    emit dataChanged(index(row, EnabledCol), index(row, EnabledCol));
}

void SetsModel::toggleRow(int row)
{
    CardSetPtr tmp = sets.at(row);

    if (tmp == nullptr)
        return;

    if (enabledSets.contains(tmp))
        enabledSets.remove(tmp);
    else
        enabledSets.insert(tmp);

    emit dataChanged(index(row, EnabledCol), index(row, EnabledCol));
}

void SetsModel::toggleAll(bool enabled)
{
    enabledSets.clear();

    if (enabled)
        for (CardSetPtr set : sets)
            enabledSets.insert(set);

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void SetsModel::swapRows(int oldRow, int newRow)
{
    beginRemoveRows(QModelIndex(), oldRow, oldRow);
    CardSetPtr temp = sets.takeAt(oldRow);
    endRemoveRows();

    beginInsertRows(QModelIndex(), newRow, newRow);
    sets.insert(newRow, temp);
    endInsertRows();

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void SetsModel::sort(int column, Qt::SortOrder order)
{
    QMultiMap<QString, CardSetPtr> setMap;
    int numRows = rowCount();
    int row;

    for (row = 0; row < numRows; ++row)
        setMap.insert(index(row, column).data(SetsModel::SortRole).toString(), sets.at(row));

    QList<CardSetPtr> tmp = setMap.values();
    sets.clear();
    if (order == Qt::AscendingOrder) {
        for (row = 0; row < tmp.size(); row++) {
            sets.append(tmp.at(row));
        }
    } else {
        for (row = tmp.size() - 1; row >= 0; row--) {
            sets.append(tmp.at(row));
        }
    }

    emit dataChanged(index(0, 0), index(numRows - 1, columnCount() - 1));
}

void SetsModel::save(CardDatabase *db)
{
    // order
    for (int i = 0; i < sets.size(); i++)
        sets[i]->setSortKey(static_cast<unsigned int>(i + 1));

    // enabled sets
    for (const CardSetPtr &set : sets)
        set->setEnabled(enabledSets.contains(set));

    sets.sortByKey();

    db->notifyEnabledSetsChanged();
}

void SetsModel::restore(CardDatabase *db)
{
    // order
    sets = db->getSetList();
    sets.sortByKey();

    // enabled sets
    enabledSets.clear();
    for (const CardSetPtr &set : sets) {
        if (set->getEnabled())
            enabledSets.insert(set);
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

QStringList SetsModel::mimeTypes() const
{
    return QStringList() << "application/x-cockatricecardset";
}

SetsDisplayModel::SetsDisplayModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

void SetsDisplayModel::fetchMore(const QModelIndex &index)
{
    int itemsToFetch = sourceModel()->rowCount(index);

    beginInsertRows(QModelIndex(), 0, itemsToFetch - 1);

    endInsertRows();
}

bool SetsDisplayModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto typeIndex = sourceModel()->index(sourceRow, SetsModel::SetTypeCol, sourceParent);
    auto nameIndex = sourceModel()->index(sourceRow, SetsModel::LongNameCol, sourceParent);
    auto shortNameIndex = sourceModel()->index(sourceRow, SetsModel::ShortNameCol, sourceParent);

    return (sourceModel()->data(typeIndex).toString().contains(filterRegExp()) ||
            sourceModel()->data(nameIndex).toString().contains(filterRegExp()) ||
            sourceModel()->data(shortNameIndex).toString().contains(filterRegExp()));
}

bool SetsDisplayModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString leftString = sourceModel()->data(left, SetsModel::SortRole).toString();
    QString rightString = sourceModel()->data(right, SetsModel::SortRole).toString();

    return QString::localeAwareCompare(leftString, rightString) < 0;
}
