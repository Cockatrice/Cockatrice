/**
 * @file settings_search_model.cpp
 * @brief Implementation of the settings search list model
 * @ingroup Dialogs
 */
#include "settings_search_model.h"

#include <QSet>

SettingsSearchModel::SettingsSearchModel(QObject *parent) : QAbstractListModel(parent)
{
}

void SettingsSearchModel::setSourceEntries(const QList<SettingsSearchEntry> &entries)
{
    beginResetModel();
    sourceEntries = entries;
    endResetModel();
    rebuildFilter();
}

void SettingsSearchModel::setFilterString(const QString &text)
{
    filterActive = !text.trimmed().isEmpty();
    if (filterActive) {
        filterQuery = text.trimmed();
        filterRegex =
            QRegularExpression(QRegularExpression::escape(filterQuery), QRegularExpression::CaseInsensitiveOption);
    }
    rebuildFilter();
}

bool SettingsSearchModel::isFilterActive() const
{
    return filterActive;
}

/**
 * @brief Calculates a relevance score for a single entry against the query
 *
 * Scoring priorities (highest to lowest):
 * 1. Label starts with query -> 100
 * 2. Label contains query    -> 80
 * 3. Group title starts with query -> 60
 * 4. Group title contains query    -> 40
 * 5. Full text regex match   -> 20
 * 6. No match                -> 0 (excluded from results)
 */
static int relevanceScore(const SettingsSearchEntry &entry, const QString &query, const QRegularExpression &regex)
{
    QString lowerQuery = query.toLower();

    // Label matches are most relevant
    QString label = entry.widgetLabel.toLower();
    if (label.startsWith(lowerQuery)) {
        return 100;
    }
    if (label.contains(lowerQuery)) {
        return 80;
    }

    // Group title matches are next
    QString group = entry.groupTitle.toLower();
    if (group.startsWith(lowerQuery)) {
        return 60;
    }
    if (group.contains(lowerQuery)) {
        return 40;
    }

    // Full text match is least relevant
    if (entry.fullSearchText.contains(regex)) {
        return 20;
    }

    return 0;
}

void SettingsSearchModel::rebuildFilter()
{
    beginResetModel();
    filteredIndices.clear();

    if (!filterActive) {
        for (int i = 0; i < sourceEntries.size(); ++i) {
            filteredIndices.append(i);
        }
    } else {
        QList<QPair<int, int>> scored; // <score, index>
        for (int i = 0; i < sourceEntries.size(); ++i) {
            int score = relevanceScore(sourceEntries[i], filterQuery, filterRegex);
            if (score > 0) {
                scored.append({-score, i}); // negative for descending sort
            }
        }
        std::sort(scored.begin(), scored.end());
        for (const auto &pair : scored) {
            filteredIndices.append(pair.second);
        }
    }
    endResetModel();
}

int SettingsSearchModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return filteredIndices.size();
}

QVariant SettingsSearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= filteredIndices.size()) {
        return {};
    }

    const SettingsSearchEntry &entry = sourceEntries[filteredIndices[index.row()]];

    switch (role) {
        case EntryRole:
            return QVariant::fromValue(entry);
        case PageIndexRole:
            return entry.pageIndex;
        case GroupTitleRole:
            return entry.groupTitle;
        case WidgetLabelRole:
            return entry.widgetLabel;
        case Qt::DisplayRole:
            return entry.widgetLabel;
        case Qt::ToolTipRole:
            return QStringLiteral("%1 > %2 > %3")
                .arg(QString::number(entry.pageIndex), entry.groupTitle, entry.widgetLabel);
        default:
            return {};
    }
}

SettingsSearchEntry SettingsSearchModel::entryForIndex(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= filteredIndices.size()) {
        return {};
    }
    return sourceEntries[filteredIndices[index.row()]];
}

QList<int> SettingsSearchModel::filteredPageIndices() const
{
    QSet<int> pages;
    for (int idx : filteredIndices) {
        pages.insert(sourceEntries[idx].pageIndex);
    }
    QList<int> result = pages.values();
    std::sort(result.begin(), result.end());
    return result;
}
