/**
 * @file visual_deck_storage_sort_filter_proxy_model.h
 * @ingroup VisualDeckStorageWidgets
 * @brief Sorting and filtering proxy on top of VisualDeckStorageModel.
 *
 * Owns all search / tag / color filter state and the sort order. Filtering is
 * evaluated against the model's data (never against widgets), so it can run
 * before any view exists and re-evaluate whenever deck data finishes loading.
 *
 * Rows are never removed by filtering. Instead, every row carries the
 * FilterMatchRole, which views read to show or hide their widgets while keeping
 * them alive; all rows stay in the proxy so they keep their sorted position.
 */

#ifndef VISUAL_DECK_STORAGE_SORT_FILTER_PROXY_MODEL_H
#define VISUAL_DECK_STORAGE_SORT_FILTER_PROXY_MODEL_H

#include "visual_deck_storage_model.h"

#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>

class VisualDeckStorageSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /**
     * @brief The order in which decks are sorted. Values must match the
     * entries of the sort widget's combo box and the stored settings value.
     */
    enum SortOrder
    {
        ByName,
        Alphabetical,
        ByLastModified,
        ByLastLoaded,
    };
    Q_ENUM(SortOrder)

    /**
     * @brief How the color identity filter is applied.
     */
    enum FilterMode
    {
        ExactMatch,
        Includes,
        Excludes
    };
    Q_ENUM(FilterMode)

    explicit VisualDeckStorageSortFilterProxyModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    void setSourceModel(QAbstractItemModel *model) override;

    /// @name Filter input setters (each re-evaluates the affected matches)
    ///@{
    void setSearchText(const QString &text);
    void setTagFilter(const QSet<QString> &newSelectedTags, const QSet<QString> &newExcludedTags);
    void setColorFilter(FilterMode mode, const QSet<QChar> &colors);
    ///@}

    /**
     * @brief Sets the sort order and applies it immediately.
     */
    void setSortOrder(SortOrder order);

    /**
     * @brief Re-evaluates all matches against the current model data and
     * re-applies filtering and sorting. Called after deck data changes.
     */
    void reapplyFilters();

    /**
     * @brief Re-applies the current sort order without touching the filters.
     */
    void resort();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    [[nodiscard]] bool rowMatches(int sourceRow) const;
    void resizeMatchLists();
    void updateSearchMatches();
    void updateTagMatches();
    void updateColorMatches();
    [[nodiscard]] const VisualDeckStorageModel *deckSourceModel() const;

    QString searchText;
    QSet<QString> selectedTags;
    QSet<QString> excludedTags;
    FilterMode colorFilterMode = ExactMatch;
    QSet<QChar> activeColors;
    SortOrder sortOrder = Alphabetical;

    QList<bool> searchMatches; ///< Per-row search match, sized like the source model.
    QList<bool> tagMatches;    ///< Per-row tag match.
    QList<bool> colorMatches;  ///< Per-row color identity match.
};

#endif // VISUAL_DECK_STORAGE_SORT_FILTER_PROXY_MODEL_H
