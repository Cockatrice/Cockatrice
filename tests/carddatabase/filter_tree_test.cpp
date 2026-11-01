#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/card_localization.h>
#include <libcockatrice/filters/filter_tree.h>
#include <gtest/gtest.h>

namespace
{
CardInfoPtr card(const QString &colors, const QString &identity)
{
    return CardInfo::newInstance("Card", "", false, {{"colors", colors}, {"coloridentity", identity}}, {}, {}, {}, {});
}
bool accepts(const FilterTree &tree, const CardInfoPtr &info)
{
    return tree.acceptsCard(info, CardSearchLanguage{});
}
} // namespace

TEST(FilterTreeIdentity, NormalizedTermIsContainedInIdentity)
{
    FilterTree tree;
    tree.termNode(CardFilter::AttrColorIdentity, CardFilter::TypeAnd, "white blue");
    EXPECT_TRUE(accepts(tree, card("W", "WU")));
    EXPECT_TRUE(accepts(tree, card("W", "WUBRG")));
    EXPECT_FALSE(accepts(tree, card("WU", "W")));
}

TEST(FilterTreeIdentity, ColorlessAliasesAndActualColors)
{
    for (const auto &term : {"c", "colorless", "colourless", "none"}) {
        FilterTree tree;
        tree.termNode(CardFilter::AttrColorIdentity, CardFilter::TypeAnd, term);
        EXPECT_TRUE(accepts(tree, card("", "")));
        EXPECT_FALSE(accepts(tree, card("", "W")));
    }
    FilterTree colors;
    colors.termNode(CardFilter::AttrColor, CardFilter::TypeAndNot, "blue");
    EXPECT_TRUE(accepts(colors, card("W", "WU")));
}

TEST(FilterTreeIdentity, ExcludesEachDisallowedColor)
{
    FilterTree tree;
    for (const auto &term : {"B", "R", "G"}) {
        tree.termNode(CardFilter::AttrColorIdentity, CardFilter::TypeAndNot, term);
    }
    EXPECT_TRUE(accepts(tree, card("W", "WU")));
    EXPECT_FALSE(accepts(tree, card("W", "WUB")));
    EXPECT_FALSE(accepts(tree, card("W", "WUBRG")));
    EXPECT_TRUE(accepts(tree, card("", "")));
}
