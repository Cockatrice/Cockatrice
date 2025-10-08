#include "card_search_model.h"

#include "../card_database_display_model.h"
#include "../card_database_model.h"

#include <algorithm>
#include <libcockatrice/utility/levenshtein.h>

CardSearchModel::CardSearchModel(CardDatabaseDisplayModel *sourceModel, QObject *parent)
    : QAbstractListModel(parent), sourceModel(sourceModel)
{
}

int CardSearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return searchResults.size();
}

QVariant CardSearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= searchResults.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return searchResults.at(index.row()).card->getName();
    }

    return QVariant();
}

void CardSearchModel::updateSearchResults(const QString &query)
{
    beginResetModel();
    searchResults.clear();

    if (query.isEmpty() || !sourceModel)
        return;

    // Set the filter for the display model
    sourceModel->setCardName(query);

    // Collect matching cards and compute Levenshtein distance
    for (int i = 0; i < sourceModel->rowCount(); ++i) {
        QModelIndex modelIndex = sourceModel->index(i, 0);
        QModelIndex sourceIndex = sourceModel->mapToSource(modelIndex);
        CardDatabaseModel *sourceDbModel = qobject_cast<CardDatabaseModel *>(sourceModel->sourceModel());

        if (!sourceDbModel || !sourceIndex.isValid())
            return;

        CardInfoPtr card = sourceDbModel->getCard(sourceIndex.row());

        if (!card)
            continue;

        int distance = levenshteinDistance(query.toLower(), card->getName().toLower());
        searchResults.append({card, distance});
    }

    // Sort by Levenshtein distance (lower distance = better match)
    std::sort(searchResults.begin(), searchResults.end(),
              [](const SearchResult &a, const SearchResult &b) { return a.distance < b.distance; });

    // Keep only the top 5 results
    if (searchResults.size() > 10)
        searchResults = searchResults.mid(0, 10);

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    emit layoutChanged();

    endResetModel();
}
