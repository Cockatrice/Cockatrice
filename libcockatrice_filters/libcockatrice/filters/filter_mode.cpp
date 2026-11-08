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

FilterMode deriveFilterMode(const QList<const CardFilter *> &filters, FilterMode currentMode)
{
    constexpr int modeCount = static_cast<int>(FilterMode::End);
    bool modeImpossible[modeCount] = {};
    const QString colors = "WUBRG";

    for (int i = 0; i < modeCount; ++i) {
        const FilterModeInfo modeInfo = getFilterModeInfo(static_cast<FilterMode>(i));

        // Remove the modes that cannot create one of the filters.
        for (const CardFilter *filter : filters) {
            if (filter->attr() != modeInfo.attr ||
                (filter->type() != modeInfo.includeType && filter->type() != modeInfo.excludeType)) {
                modeImpossible[i] = true;
                break;
            }
        }

        // Modes that have both an include and an exclude filter should always have a filter for each color.
        if (modeInfo.includeType != CardFilter::Type::TypeEnd && modeInfo.excludeType != CardFilter::Type::TypeEnd &&
            !modeImpossible[i]) {
            for (const QChar &color : colors) {
                int count = 0;
                for (const CardFilter *filter : filters) {
                    count += filter->term() == color;
                }
                if (count != 1) {
                    modeImpossible[i] = true;
                    break;
                }
            }
        }
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
