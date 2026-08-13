#include "visual_deck_storage_sort_filter_proxy_model.h"

#include "../../filters/deck_filter_string.h"

#include <QFileInfo>

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
    if (searchText_ == text) {
        return;
    }

    searchText_ = text;
    updateSearchMatches();
    invalidate();
}

void VisualDeckStorageSortFilterProxyModel::setTagFilter(const QSet<QString> &selectedTags,
                                                         const QSet<QString> &excludedTags)
{
    if (selectedTags_ == selectedTags && excludedTags_ == excludedTags) {
        return;
    }

    selectedTags_ = selectedTags;
    excludedTags_ = excludedTags;
    updateTagMatches();
    invalidate();
}

void VisualDeckStorageSortFilterProxyModel::setColorFilter(FilterMode mode, const QSet<QChar> &activeColors)
{
    if (colorFilterMode_ == mode && activeColors_ == activeColors) {
        return;
    }

    colorFilterMode_ = mode;
    activeColors_ = activeColors;
    updateColorMatches();
    invalidate();
}

void VisualDeckStorageSortFilterProxyModel::setSortOrder(SortOrder order)
{
    // No equality guard: the initial reapply (with the default order) must still
    // trigger sort(0), since without a sort the proxy would show scan order.
    sortOrder_ = order;
    sort(0);
}

void VisualDeckStorageSortFilterProxyModel::reapplyFilters()
{
    const QList<bool> oldSearchMatches = searchMatches_;
    const QList<bool> oldTagMatches = tagMatches_;
    const QList<bool> oldColorMatches = colorMatches_;

    updateSearchMatches();
    updateTagMatches();
    updateColorMatches();

    if (searchMatches_ != oldSearchMatches || tagMatches_ != oldTagMatches || colorMatches_ != oldColorMatches) {
        invalidate();
    }

    if (sortOrder_ == ByName || sortOrder_ == ByLastLoaded) {
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
    if (sourceRow < 0 || sourceRow >= searchMatches_.size() || sourceRow >= tagMatches_.size() ||
        sourceRow >= colorMatches_.size()) {
        return true;
    }

    return searchMatches_.at(sourceRow) && tagMatches_.at(sourceRow) && colorMatches_.at(sourceRow);
}

bool VisualDeckStorageSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto *source = deckSourceModel();
    if (!source) {
        return false;
    }

    const DeckPreviewData &leftData = source->dataForRow(left.row());
    const DeckPreviewData &rightData = source->dataForRow(right.row());

    switch (sortOrder_) {
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
    searchMatches_ = QList<bool>(count, true);
    tagMatches_ = QList<bool>(count, true);
    colorMatches_ = QList<bool>(count, true);
}

void VisualDeckStorageSortFilterProxyModel::updateSearchMatches()
{
    const auto *source = deckSourceModel();
    if (!source) {
        searchMatches_.clear();
        return;
    }

    const int count = source->rowCount();
    if (searchText_.isEmpty()) {
        searchMatches_ = QList<bool>(count, true);
        return;
    }

    searchMatches_.resize(count);
    DeckFilterString filterString(searchText_);
    for (int row = 0; row < count; ++row) {
        const DeckPreviewData &data = source->dataForRow(row);

        // Decks that haven't finished loading yet can't be evaluated; show them and
        // re-evaluate once the load finishes (reapplyFilters is triggered by deckLoaded).
        if (data.deck.isEmpty()) {
            searchMatches_[row] = true;
            continue;
        }

        DeckSearchData searchData;
        searchData.deck = &data.deck;
        searchData.filePath = data.filePath;
        searchData.displayName = data.displayName;
        searchData.relativeFilePath = data.relativeFilePath;
        searchMatches_[row] = filterString.check(searchData);
    }
}

void VisualDeckStorageSortFilterProxyModel::updateTagMatches()
{
    const auto *source = deckSourceModel();
    if (!source) {
        tagMatches_.clear();
        return;
    }

    const int count = source->rowCount();
    tagMatches_.resize(count);

    if (selectedTags_.isEmpty() && excludedTags_.isEmpty()) {
        tagMatches_.fill(true);
        return;
    }

    for (int row = 0; row < count; ++row) {
        const QStringList deckTags = source->dataForRow(row).tags;

        bool hasAllSelected = true;
        for (const QString &tag : selectedTags_) {
            if (!deckTags.contains(tag)) {
                hasAllSelected = false;
                break;
            }
        }

        bool hasAnyExcluded = false;
        for (const QString &tag : excludedTags_) {
            if (deckTags.contains(tag)) {
                hasAnyExcluded = true;
                break;
            }
        }

        tagMatches_[row] = hasAllSelected && !hasAnyExcluded;
    }
}

void VisualDeckStorageSortFilterProxyModel::updateColorMatches()
{
    const auto *source = deckSourceModel();
    if (!source) {
        colorMatches_.clear();
        return;
    }

    const int count = source->rowCount();
    colorMatches_.resize(count);

    if (activeColors_.isEmpty()) {
        colorMatches_.fill(true);
        return;
    }

    for (int row = 0; row < count; ++row) {
        const QString colorIdentity = source->dataForRow(row).colorIdentity;

        bool matches = true;
        switch (colorFilterMode_) {
            case ExactMatch: {
                QSet<QChar> activeColorSet;
                for (const QChar &color : activeColors_) {
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
                for (const QChar &color : activeColors_) {
                    if (!colorIdentity.contains(color)) {
                        matches = false;
                        break;
                    }
                }
                break;
            case Excludes:
                for (const QChar &color : activeColors_) {
                    if (colorIdentity.contains(color)) {
                        matches = false;
                        break;
                    }
                }
                break;
        }

        colorMatches_[row] = matches;
    }
}

const VisualDeckStorageModel *VisualDeckStorageSortFilterProxyModel::deckSourceModel() const
{
    return qobject_cast<const VisualDeckStorageModel *>(sourceModel());
}
