#include "card_search_model.h"

#include "../card_database_display_model.h"
#include "../card_database_model.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QVariant>
#include <QtGlobal>
#include <algorithm>
#include <libcockatrice/utility/levenshtein.h>
#include <utility>

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
    if (!index.isValid() || index.row() >= searchResults.size()) {
        return QVariant();
    }

    const SearchResult &result = searchResults.at(index.row());

    if (role == Qt::DisplayRole) {
        return result.card->getName();
    }

    if (role == CardInfoRole) {
        return QVariant::fromValue(result.card);
    }

    return QVariant();
}

void CardSearchModel::updateSearchResults(const QString &query)
{
    beginResetModel();
    searchResults.clear();

    if (query.isEmpty() || !sourceModel) {
        endResetModel();
        return;
    }

    CardDatabaseModel *sourceDbModel = qobject_cast<CardDatabaseModel *>(sourceModel->sourceModel());
    if (!sourceDbModel) {
        endResetModel();
        return;
    }

    const QString lowerQuery = query.toLower();

    QList<SearchResult> prefixMatches;
    QList<SearchResult> containsMatches;

    // Iterate the raw database model directly so results are always complete and fresh
    const int rowCount = sourceDbModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        CardInfoPtr card = sourceDbModel->getCard(i);
        if (!card) {
            continue;
        }

        const std::optional<QString> lowerName = matchingLowercaseName(card, lowerQuery);
        if (!lowerName) {
            continue;
        }

        const int distance = levenshteinDistance(lowerQuery, *lowerName);
        // Length of the matched name, used to break distance ties. Stored here
        // so the comparator does not look the name up again for every comparison.
        const int nameLength = static_cast<int>(lowerName->size());

        if (lowerName->startsWith(lowerQuery)) {
            prefixMatches.append({card, distance, nameLength});
        } else {
            containsMatches.append({card, distance, nameLength});
        }
    }

    auto sortByDistanceThenLength = [](const SearchResult &a, const SearchResult &b) {
        if (a.distance != b.distance) {
            return a.distance < b.distance;
        }
        return a.nameLength < b.nameLength;
    };

    std::sort(prefixMatches.begin(), prefixMatches.end(), sortByDistanceThenLength);
    std::sort(containsMatches.begin(), containsMatches.end(), sortByDistanceThenLength);

    // Prefix matches always come first, then contains-only matches
    searchResults.reserve(prefixMatches.size() + containsMatches.size());
    searchResults.append(prefixMatches);
    searchResults.append(containsMatches);

    // Keep only the top 10 results
    if (searchResults.size() > 10) {
        searchResults = searchResults.mid(0, 10);
    }

    endResetModel();
}

std::optional<QString> CardSearchModel::matchingLowercaseName(const CardInfoPtr &card, const QString &lowerQuery) const
{
    if (!searchLanguage.isEnglishOnly() && searchLanguage.mode == SearchLanguageMode::Selected) {
        QString localizedName = card->getLocalizedName(searchLanguage.language).toLower();
        if (localizedName.contains(lowerQuery)) {
            return std::optional<QString>(std::move(localizedName));
        }
        return std::nullopt;
    }

    QString englishName = card->getName().toLower();
    if (englishName.contains(lowerQuery)) {
        return std::optional<QString>(std::move(englishName));
    }

    if (searchLanguage.isEnglishOnly()) {
        return std::nullopt;
    }

    QString localizedName = card->getLocalizedName(searchLanguage.language).toLower();
    if (localizedName.contains(lowerQuery)) {
        return std::optional<QString>(std::move(localizedName));
    }
    return std::nullopt;
}
