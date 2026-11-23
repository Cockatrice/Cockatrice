/**
 * @file filter_mode.cpp
 * @ingroup Filters
 */

#include "libcockatrice/filters/filter_mode.h"

#include <QString>

FilterModeInfo getFilterModeInfo(FilterMode filterMode)
{
    switch (filterMode) {
        case FilterMode::Includes:
            return {CardFilter::Type::TypeOr, CardFilter::Type::TypeEnd, CardFilter::Attr::AttrColor};
        case FilterMode::Excludes:
            return {CardFilter::Type::TypeAndNot, CardFilter::Type::TypeEnd, CardFilter::Attr::AttrColor};
        case FilterMode::IncludeExclude:
            return {CardFilter::Type::TypeOr, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor};
        case FilterMode::ExactMatch:
            return {CardFilter::Type::TypeAnd, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor};
        case FilterMode::ColorIdentity:
            return {CardFilter::Type::TypeEnd, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColorIdentity};
        case FilterMode::Custom:
        case FilterMode::End:
            break;
    }
    return {CardFilter::Type::TypeEnd, CardFilter::Type::TypeEnd, CardFilter::Attr::AttrEnd};
}

namespace
{
/// Whether every filter in the set could have been created by this mode.
bool canCreateEveryFilter(const FilterModeInfo &modeInfo, const QList<const CardFilter *> &filters)
{
    for (const CardFilter *filter : filters) {
        const bool createdByMode = filter->attr() == modeInfo.attr &&
                                   (filter->type() == modeInfo.includeType || filter->type() == modeInfo.excludeType);
        if (!createdByMode) {
            return false;
        }
    }
    return true;
}

/// Modes with both an include and an exclude filter need exactly one filter per color.
bool hasOneFilterPerColor(const FilterModeInfo &modeInfo, const QList<const CardFilter *> &filters)
{
    if (modeInfo.includeType == CardFilter::Type::TypeEnd || modeInfo.excludeType == CardFilter::Type::TypeEnd) {
        return true;
    }

    for (const QChar &color : QStringLiteral("WUBRG")) {
        int count = 0;
        for (const CardFilter *filter : filters) {
            if (filter->term() == color) {
                ++count;
            }
        }
        if (count != 1) {
            return false;
        }
    }
    return true;
}
} // namespace

FilterMode deriveFilterMode(const QList<const CardFilter *> &filters, FilterMode currentMode)
{
    constexpr int modeCount = static_cast<int>(FilterMode::End);
    bool modeImpossible[modeCount] = {};

    for (int i = 0; i < modeCount; ++i) {
        const FilterModeInfo modeInfo = getFilterModeInfo(static_cast<FilterMode>(i));

        // A mode is impossible when it cannot create every filter, or when it has both
        // an include and an exclude filter and a color is not covered exactly once.
        modeImpossible[i] = !canCreateEveryFilter(modeInfo, filters) || !hasOneFilterPerColor(modeInfo, filters);
    }

    // Keep the current mode if it can produce the filters, so that a live mode is never swapped for another.
    // Switching to and from the custom mode is always possible.
    if (!modeImpossible[static_cast<int>(currentMode)] && currentMode != FilterMode::Custom) {
        return currentMode;
    }

    for (int i = 0; i < modeCount; ++i) {
        if (!modeImpossible[i] || i == static_cast<int>(FilterMode::Custom)) {
            return static_cast<FilterMode>(i);
        }
    }

    return FilterMode::Custom;
}
