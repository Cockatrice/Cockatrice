#include "visual_deck_storage_sort_filter_proxy_model.h"

#include "../../filters/deck_filter_string.h"

#include <QFileInfo>
#include <algorithm>

VisualDeckStorageSortFilterProxyModel::VisualDeckStorageSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(false);
}

void VisualDeckStorageSortFilterProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (QAbstractItemModel *oldModel = sourceModel()) {
        disconnect(oldModel, &QAbstractItemModel::modelReset, this,
                   &VisualDeckStorageSortFilterProxyModel::resizeMatchLists);
        disconnect(oldModel, &QAbstractItemModel::rowsInserted, this,
                   &VisualDeckStorageSortFilterProxyModel::resizeMatchLists);
        disconnect(oldModel, &QAbstractItemModel::rowsRemoved, this,
                   &VisualDeckStorageSortFilterProxyModel::resizeMatchLists);
    }

    QSortFilterProxyModel::setSourceModel(model);

    if (model) {
        connect(model, &QAbstractItemModel::modelReset, this, &VisualDeckStorageSortFilterProxyModel::resizeMatchLists);
        connect(model, &QAbstractItemModel::rowsInserted, this,
                &VisualDeckStorageSortFilterProxyModel::resizeMatchLists);
        connect(model, &QAbstractItemModel::rowsRemoved, this,
                &VisualDeckStorageSortFilterProxyModel::resizeMatchLists);
    }

    resizeMatchLists();
}

void VisualDeckStorageSortFilterProxyModel::setSearchText(const QString &text)
{
    if (searchText == text) {
        return;
    }

    searchText = text;
    updateSearchMatches();
    invalidate();
}

void VisualDeckStorageSortFilterProxyModel::setTagFilter(const QSet<QString> &newSelectedTags,
                                                         const QSet<QString> &newExcludedTags)
{
    if (selectedTags == newSelectedTags && excludedTags == newExcludedTags) {
        return;
    }

    selectedTags = newSelectedTags;
    excludedTags = newExcludedTags;
    updateTagMatches();
    invalidate();
}

void VisualDeckStorageSortFilterProxyModel::setColorFilter(FilterMode mode, const QSet<QChar> &colors)
{
    if (colorFilterMode == mode && activeColors == colors) {
        return;
    }

    colorFilterMode = mode;
    activeColors = colors;
    updateColorMatches();
    invalidate();
}

void VisualDeckStorageSortFilterProxyModel::setSortOrder(SortOrder order)
{
    // No equality guard: the initial reapply (with the default order) must still
    // trigger sort(0), since without a sort the proxy would show scan order.
    sortOrder = order;
    sort(0);
}

void VisualDeckStorageSortFilterProxyModel::reapplyFilters()
{
    const QList<bool> oldSearchMatches = searchMatches;
    const QList<bool> oldTagMatches = tagMatches;
    const QList<bool> oldColorMatches = colorMatches;

    updateSearchMatches();
    updateTagMatches();
    updateColorMatches();

    if (searchMatches != oldSearchMatches || tagMatches != oldTagMatches || colorMatches != oldColorMatches) {
        invalidate();
    }

    if (sortOrder == ByName || sortOrder == ByLastLoaded) {
        // These orders depend on data that only becomes available when a deck finishes loading.
        sort(0);
    }
}

void VisualDeckStorageSortFilterProxyModel::resort()
{
    sort(0);
}

bool VisualDeckStorageSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceParent.isValid()) {
        return true;
    }

    // If the match lists aren't sized to the current model yet, don't hide anything.
    if (sourceRow < 0 || sourceRow >= searchMatches.size() || sourceRow >= tagMatches.size() ||
        sourceRow >= colorMatches.size()) {
        return true;
    }

    return searchMatches.at(sourceRow) && tagMatches.at(sourceRow) && colorMatches.at(sourceRow);
}

bool VisualDeckStorageSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto *source = deckSourceModel();
    if (!source) {
        return false;
    }

    const DeckPreviewData &leftData = source->dataForRow(left.row());
    const DeckPreviewData &rightData = source->dataForRow(right.row());

    switch (sortOrder) {
        case ByName:
            return leftData.deckName < rightData.deckName;
        case Alphabetical:
            return QString::localeAwareCompare(QFileInfo(leftData.filePath).fileName(),
                                               QFileInfo(rightData.filePath).fileName()) < 0;
        case ByLastModified:
            return leftData.lastModified > rightData.lastModified;
        case ByLastLoaded:
            return leftData.lastLoaded > rightData.lastLoaded;
    }

    return false;
}

void VisualDeckStorageSortFilterProxyModel::resizeMatchLists()
{
    const int count = sourceModel() ? sourceModel()->rowCount() : 0;
    searchMatches.resize(count);
    searchMatches.fill(true);
    tagMatches.resize(count);
    tagMatches.fill(true);
    colorMatches.resize(count);
    colorMatches.fill(true);
}

void VisualDeckStorageSortFilterProxyModel::updateSearchMatches()
{
    const auto *source = deckSourceModel();
    if (!source) {
        searchMatches.clear();
        return;
    }

    const int count = source->rowCount();
    searchMatches.resize(count);
    if (searchText.isEmpty()) {
        searchMatches.fill(true);
        return;
    }

    DeckFilterString filterString(searchText);
    for (int row = 0; row < count; ++row) {
        const DeckPreviewData &data = source->dataForRow(row);

        // isEmpty() is intentional: if a deck fails to load, loadInProgress becomes false
        // but the deck remains empty. Using loadInProgress alone would pass failed decks
        // to DeckFilterString::check, which requires a non-empty deck.
        if (data.deck.isEmpty()) {
            searchMatches[row] = true;
            continue;
        }

        DeckSearchData searchData{
            .deck = &data.deck,
            .filePath = data.filePath,
            .displayName = data.displayName,
            .relativeFilePath = data.relativeFilePath,
        };
        searchMatches[row] = filterString.check(searchData);
    }
}

void VisualDeckStorageSortFilterProxyModel::updateTagMatches()
{
    const auto *source = deckSourceModel();
    if (!source) {
        tagMatches.clear();
        return;
    }

    const int count = source->rowCount();
    tagMatches.resize(count);

    if (selectedTags.isEmpty() && excludedTags.isEmpty()) {
        tagMatches.fill(true);
        return;
    }

    for (int row = 0; row < count; ++row) {
        const QStringList deckTags = source->dataForRow(row).tags;

        const bool hasAllSelected = std::all_of(selectedTags.begin(), selectedTags.end(),
                                                [&deckTags](const QString &tag) { return deckTags.contains(tag); });
        const bool hasAnyExcluded = std::any_of(excludedTags.begin(), excludedTags.end(),
                                                [&deckTags](const QString &tag) { return deckTags.contains(tag); });

        tagMatches[row] = hasAllSelected && !hasAnyExcluded;
    }
}

void VisualDeckStorageSortFilterProxyModel::updateColorMatches()
{
    const auto *source = deckSourceModel();
    if (!source) {
        colorMatches.clear();
        return;
    }

    const int count = source->rowCount();
    colorMatches.resize(count);

    if (activeColors.isEmpty()) {
        colorMatches.fill(true);
        return;
    }

    for (int row = 0; row < count; ++row) {
        const QString colorIdentity = source->dataForRow(row).colorIdentity;

        bool matches = true;
        switch (colorFilterMode) {
            case ExactMatch: {
                QSet<QChar> activeColorSet;
                for (const QChar &color : activeColors) {
                    activeColorSet.insert(color.toUpper());
                }

                QSet<QChar> colorIdentitySet;
                for (const QChar &color : colorIdentity) {
                    colorIdentitySet.insert(color.toUpper());
                }

                matches = activeColorSet == colorIdentitySet;
                break;
            }
            case Includes:
                matches = std::all_of(activeColors.begin(), activeColors.end(),
                                      [&colorIdentity](const QChar &color) { return colorIdentity.contains(color); });
                break;
            case Excludes:
                matches = std::none_of(activeColors.begin(), activeColors.end(),
                                       [&colorIdentity](const QChar &color) { return colorIdentity.contains(color); });
                break;
        }

        colorMatches[row] = matches;
    }
}

const VisualDeckStorageModel *VisualDeckStorageSortFilterProxyModel::deckSourceModel() const
{
    return qobject_cast<const VisualDeckStorageModel *>(sourceModel());
}
