#include "reversed_completer_model.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

void ReversedCompleterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel == this->sourceModel()) {
        return;
    }

    if (QAbstractItemModel *old = this->sourceModel()) {
        disconnect(old, nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::rowsMoved, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::columnsInserted, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::columnsRemoved, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::dataChanged, this, &ReversedCompleterModel::invalidate);
        connect(sourceModel, &QAbstractItemModel::layoutChanged, this, &ReversedCompleterModel::invalidate);
    }

    invalidate();
}

void ReversedCompleterModel::setEnabled(bool enabled)
{
    if (enabled == isEnabled) {
        return;
    }
    isEnabled = enabled;
    invalidate();
}

QModelIndex ReversedCompleterModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid() || !sourceModel()) {
        return {};
    }
    const int sourceRow = isEnabled ? sourceRowCount() - 1 - proxyIndex.row() : proxyIndex.row();
    return sourceModel()->index(sourceRow, proxyIndex.column());
}

QModelIndex ReversedCompleterModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid() || !sourceModel()) {
        return {};
    }
    const int proxyRow = isEnabled ? sourceRowCount() - 1 - sourceIndex.row() : sourceIndex.row();
    return index(proxyRow, sourceIndex.column());
}

QModelIndex ReversedCompleterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || !sourceModel() || row < 0 || row >= rowCount() || column < 0 || column >= columnCount()) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex ReversedCompleterModel::parent(const QModelIndex &) const
{
    return {};
}

int ReversedCompleterModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() || !sourceModel() ? 0 : sourceModel()->rowCount();
}

int ReversedCompleterModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() || !sourceModel() ? 0 : sourceModel()->columnCount();
}

QVariant ReversedCompleterModel::data(const QModelIndex &proxyIndex, int role) const
{
    return sourceModel() ? sourceModel()->data(mapToSource(proxyIndex), role) : QVariant();
}

QVariant ReversedCompleterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel() ? sourceModel()->headerData(section, orientation, role) : QVariant();
}

int ReversedCompleterModel::sourceRowCount() const
{
    return sourceModel() ? sourceModel()->rowCount() : 0;
}

void ReversedCompleterModel::invalidate()
{
    beginResetModel();
    endResetModel();
}
