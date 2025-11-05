#include "token_edit_model.h"

#include "../card_database_display_model.h"
#include "../card_database_model.h"

#include <libcockatrice/card/card_info.h>

TokenEditModel::TokenEditModel(QObject *parent) : CardDatabaseDisplayModel(parent)
{
}

bool TokenEditModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    CardInfoPtr info = static_cast<CardDatabaseModel *>(sourceModel())->getCard(sourceRow);
    return info->getIsToken() && info->getSets().contains(CardSet::TOKENS_SETNAME) && rowMatchesCardName(info);
}

int TokenEditModel::rowCount(const QModelIndex &parent) const
{
    // always load all tokens at start
    return QSortFilterProxyModel::rowCount(parent);
}