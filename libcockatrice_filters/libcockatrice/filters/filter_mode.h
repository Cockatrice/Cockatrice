/**
 * @file filter_mode.h
 * @ingroup Filters
 */

#ifndef LIBCOCKATRICE_FILTERS_FILTER_MODE_H
#define LIBCOCKATRICE_FILTERS_FILTER_MODE_H

#include "libcockatrice/filters/filter_card.h"

#include <QChar>
#include <QList>
#include <QMetaType>

/**
 * @brief The way selected and unselected colors are combined into card filters.
 */
enum class FilterMode
{
    Includes = 0,   // Include selected colors (OR condition).
    Excludes,       // Exclude selected colors (AND NOT).
    IncludeExclude, // Include selected colors (OR) and exclude unselected colors (AND NOT).
    ExactMatch,     // Only selected colors are included, all others are excluded.
    ColorIdentity,  // Keep cards whose color identity is a subset of the selected colors (stored as AND NOT for each
                    // unselected color).
    Custom,         // The color filters do not match any of the above modes. Not included in the combo box.
    End
};

Q_DECLARE_METATYPE(FilterMode)

/**
 * @brief The filters a single color contributes in a given mode.
 */
struct FilterModeInfo
{
    // When a color symbol is enabled, add filters of this type. Skipped if equal to CardFilter::Type::TypeEnd.
    CardFilter::Type includeType;

    // When a color symbol is disabled, add filters of this type. Skipped if equal to CardFilter::Type::TypeEnd.
    CardFilter::Type excludeType;

    // The attribute to filter for when adding the filters.
    CardFilter::Attr attr;
};

/** @brief Return the filters the given mode contributes for one color. */
FilterModeInfo getFilterModeInfo(FilterMode filterMode);

/**
 * @brief Determine the mode that produced the given color filters.
 *
 * Returns @p currentMode unchanged when that mode can still produce @p filters, so that a live mode is never
 * replaced by an equally valid one. Otherwise returns the first mode that can produce them, falling back to
 * FilterMode::Custom when none can.
 *
 * @param filters The filters on CardFilter::Attr::AttrColor and CardFilter::Attr::AttrColorIdentity.
 * @param currentMode The mode currently selected in the UI.
 */
FilterMode deriveFilterMode(const QList<const CardFilter *> &filters, FilterMode currentMode);

#endif // LIBCOCKATRICE_FILTERS_FILTER_MODE_H
