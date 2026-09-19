#include "card_completer_proxy_model.h"

CardCompleterProxyModel::CardCompleterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool CardCompleterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel()) {
        return false;
    }

    if (filterRegularExpression().pattern().isEmpty()) {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QString data = index.data(Qt::DisplayRole).toString();

    return data.contains(filterRegularExpression());
}
