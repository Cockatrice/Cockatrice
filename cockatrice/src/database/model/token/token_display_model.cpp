#include "token_display_model.h"

#include "../card_database_model.h"

TokenDisplayModel::TokenDisplayModel(QObject *parent) : CardDatabaseDisplayModel(parent)
{
}

bool TokenDisplayModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    CardInfoPtr info = static_cast<CardDatabaseModel *>(sourceModel())->getCard(sourceRow);
    return info->getIsToken() && rowMatchesCardName(info);
}

int TokenDisplayModel::rowCount(const QModelIndex &parent) const
{
    // always load all tokens at start
    return QSortFilterProxyModel::rowCount(parent);
}