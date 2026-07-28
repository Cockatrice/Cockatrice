/**
 * @file settings_search_model.h
 * @brief Data model for the settings search feature
 * @ingroup Dialogs
 */
#ifndef COCKATRICE_SETTINGS_SEARCH_MODEL_H
#define COCKATRICE_SETTINGS_SEARCH_MODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QRegularExpression>
#include <QWidget>

/**
 * @brief Represents a single searchable setting entry
 *
 * Each settings page provides a list of these entries via getSearchEntries().
 * The model uses them for filtering, relevance scoring, and display.
 */
struct SettingsSearchEntry
{
    int pageIndex;          ///< Index of the settings page this entry belongs to
    QString groupTitle;     ///< Title of the group/section within the page
    QString widgetLabel;    ///< Display label for the setting widget
    QString fullSearchText; ///< Extended search text for full-text matching
    QWidget *widget;        ///< Pointer to the setting widget for focus/scrolling
};

/**
 * @brief List model providing filtered, ranked search results
 *
 * Manages a list of SettingsSearchEntry items. When a filter string is set,
 * entries are scored by relevance and sorted so the best matches appear first.
 * Supports custom roles for accessing entry fields from views and delegates.
 */
class SettingsSearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * @brief Custom data roles for accessing entry fields
     */
    enum Roles
    {
        EntryRole = Qt::UserRole + 1, ///< Full SettingsSearchEntry object
        PageIndexRole,                ///< Entry's page index (int)
        GroupTitleRole,               ///< Entry's group title (QString)
        WidgetLabelRole,              ///< Entry's widget label (QString)
        HighlightStartRole,           ///< Start of highlight range
        HighlightEndRole,             ///< End of highlight range
    };

    explicit SettingsSearchModel(QObject *parent = nullptr);

    /** @brief Replaces the source entries and rebuilds the filter */
    void setSourceEntries(const QList<SettingsSearchEntry> &entries);
    /** @brief Sets the filter string and recalculates the results */
    void setFilterString(const QString &text);
    /** @brief Whether a non-empty filter is currently active */
    bool isFilterActive() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /** @brief Returns the full entry for a given model index */
    SettingsSearchEntry entryForIndex(const QModelIndex &index) const;
    /** @brief Returns sorted list of page indices present in filtered results */
    QList<int> filteredPageIndices() const;

private:
    QList<SettingsSearchEntry> sourceEntries; ///< Complete unfiltered entry list
    QList<int> filteredIndices;               ///< Indices into sourceEntries matching the filter
    QRegularExpression filterRegex;           ///< Compiled regex for the current filter
    QString filterQuery;                      ///< Current filter query string
    bool filterActive = false;                ///< Whether filtering is active

    /** @brief Recalculates the filtered index list and ranking */
    void rebuildFilter();
};

Q_DECLARE_METATYPE(SettingsSearchEntry)

#endif // COCKATRICE_SETTINGS_SEARCH_MODEL_H
