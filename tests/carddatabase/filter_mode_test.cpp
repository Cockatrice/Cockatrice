#include "libcockatrice/filters/filter_mode.h"

#include <gtest/gtest.h>

class FilterModeTest : public ::testing::Test
{
protected:
    ~FilterModeTest() override
    {
        for (CardFilter *filter : owned) {
            delete filter;
        }
    }

    /** @brief Create a filter owned by the fixture and append it to the set under test. */
    const CardFilter *
    add(const QString &term, CardFilter::Type type, CardFilter::Attr attr = CardFilter::Attr::AttrColor)
    {
        owned.append(new CardFilter(term, type, attr));
        filters.append(owned.last());
        return owned.last();
    }

    QList<CardFilter *> owned;
    QList<const CardFilter *> filters;
};

TEST(FilterModeInfoTest, EachModeDescribesItsOwnFilters)
{
    EXPECT_EQ(getFilterModeInfo(FilterMode::Includes).includeType, CardFilter::Type::TypeOr);
    EXPECT_EQ(getFilterModeInfo(FilterMode::Includes).excludeType, CardFilter::Type::TypeEnd);
    EXPECT_EQ(getFilterModeInfo(FilterMode::Includes).attr, CardFilter::Attr::AttrColor);

    EXPECT_EQ(getFilterModeInfo(FilterMode::Excludes).includeType, CardFilter::Type::TypeAndNot);
    EXPECT_EQ(getFilterModeInfo(FilterMode::Excludes).excludeType, CardFilter::Type::TypeEnd);

    EXPECT_EQ(getFilterModeInfo(FilterMode::IncludeExclude).includeType, CardFilter::Type::TypeOr);
    EXPECT_EQ(getFilterModeInfo(FilterMode::IncludeExclude).excludeType, CardFilter::Type::TypeAndNot);

    EXPECT_EQ(getFilterModeInfo(FilterMode::ExactMatch).includeType, CardFilter::Type::TypeAnd);
    EXPECT_EQ(getFilterModeInfo(FilterMode::ExactMatch).excludeType, CardFilter::Type::TypeAndNot);

    // Color identity only excludes: the selected colors must be the identity, so nothing is included positively.
    EXPECT_EQ(getFilterModeInfo(FilterMode::ColorIdentity).includeType, CardFilter::Type::TypeEnd);
    EXPECT_EQ(getFilterModeInfo(FilterMode::ColorIdentity).excludeType, CardFilter::Type::TypeAndNot);
    EXPECT_EQ(getFilterModeInfo(FilterMode::ColorIdentity).attr, CardFilter::Attr::AttrColorIdentity);

    EXPECT_EQ(getFilterModeInfo(FilterMode::Custom).attr, CardFilter::Attr::AttrEnd);
}

TEST_F(FilterModeTest, EmptyFilterSetKeepsACompatibleMode)
{
    // Includes and Color identity both allow an empty set, so the live mode is not swapped for another.
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Includes), FilterMode::Includes);
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::ColorIdentity), FilterMode::ColorIdentity);
}

// Include / Exclude and Exact match always hold one filter per color, so an empty set cannot have come from
// them and falls back to the first mode that can describe it.
TEST_F(FilterModeTest, EmptyFilterSetFallsBackWhenTheModeNeedsOneFilterPerColor)
{
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::IncludeExclude), FilterMode::Includes);
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::ExactMatch), FilterMode::Includes);
}

TEST_F(FilterModeTest, IncludesFromOrFilters)
{
    add("W", CardFilter::Type::TypeOr);

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Includes), FilterMode::Includes);
    // A single OR filter cannot come from Excludes, so the first mode that can produce it is selected.
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Excludes), FilterMode::Includes);
}

TEST_F(FilterModeTest, ExcludesFromAndNotFilters)
{
    add("W", CardFilter::Type::TypeAndNot);

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Excludes), FilterMode::Excludes);
    // Includes needs an OR or an end filter, so an AND NOT filter selects Excludes.
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Includes), FilterMode::Excludes);
}

TEST_F(FilterModeTest, IncludeExcludeNeedsOneFilterPerColor)
{
    add("W", CardFilter::Type::TypeOr);
    for (const QChar color : QString("UBRG")) {
        add(QString(color), CardFilter::Type::TypeAndNot);
    }

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::IncludeExclude), FilterMode::IncludeExclude);
}

TEST_F(FilterModeTest, IncludeExcludeRejectsAnIncompleteColorSet)
{
    add("W", CardFilter::Type::TypeOr);
    add("U", CardFilter::Type::TypeAndNot);

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::IncludeExclude), FilterMode::Custom);
}

TEST_F(FilterModeTest, ExactMatchFromAndAndAndNotFilters)
{
    add("W", CardFilter::Type::TypeAnd);
    for (const QChar color : QString("UBRG")) {
        add(QString(color), CardFilter::Type::TypeAndNot);
    }

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::ExactMatch), FilterMode::ExactMatch);
}

TEST_F(FilterModeTest, ColorIdentityFromIdentityAndNotFilters)
{
    for (const QChar color : QString("BRG")) {
        add(QString(color), CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColorIdentity);
    }

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::ColorIdentity), FilterMode::ColorIdentity);
}

TEST_F(FilterModeTest, MixedAttributesAreCustom)
{
    add("W", CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor);
    add("U", CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColorIdentity);

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Includes), FilterMode::Custom);
}

TEST_F(FilterModeTest, CustomLeavesAsSoonAsAModeCanExplainTheFilters)
{
    add("W", CardFilter::Type::TypeOr);

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Custom), FilterMode::Includes);
}

// Exact match with no color selected stores one AND NOT filter per color. That is a legitimate Exact match
// state, so the live mode is kept; a mode that cannot produce the set re-derives to Excludes, which produces
// the identical filters.
TEST_F(FilterModeTest, FullyExcludedColorSetIsExactMatchOrExcludes)
{
    for (const QChar color : QString("WUBRG")) {
        add(QString(color), CardFilter::Type::TypeAndNot);
    }

    EXPECT_EQ(deriveFilterMode(filters, FilterMode::ExactMatch), FilterMode::ExactMatch);
    EXPECT_EQ(deriveFilterMode(filters, FilterMode::Includes), FilterMode::Excludes);
}
