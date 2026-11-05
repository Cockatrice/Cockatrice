
#include "libcockatrice/filters/filter_tree.h"

#include <gtest/gtest.h>
#include <libcockatrice/card/card_localization.h>
#include <libcockatrice/filters/filter_string.h>

namespace
{
class FilterTreeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        language = CardSearchLanguage{};

        island = CardInfo::newInstance("Island", "", false, {{"colors", ""}, {"coloridentity", "U"}}, {}, {}, {}, {});
        artifact =
            CardInfo::newInstance("Artifact", "", false, {{"colors", ""}, {"coloridentity", ""}}, {}, {}, {}, {});
        truth = CardInfo::newInstance("Truth", "", false, {{"colors", "U"}, {"coloridentity", "U"}}, {}, {}, {}, {});
        notDead =
            CardInfo::newInstance("Not Dead", "", false, {{"colors", "B"}, {"coloridentity", "B"}}, {}, {}, {}, {});
        wallOfDenial = CardInfo::newInstance("Wall of Denial", "", false, {{"colors", "WU"}, {"coloridentity", "WU"}},
                                             {}, {}, {}, {});
        maelstromAngel = CardInfo::newInstance("Maelstrom Archangel", "", false,
                                               {{"colors", "WUBRG"}, {"coloridentity", "WUBRG"}}, {}, {}, {}, {});
    }

    CardSearchLanguage language;

    CardData island;         // c/u
    CardData artifact;       // c
    CardData truth;          // u
    CardData notDead;        // b
    CardData wallOfDenial;   // wu
    CardData maelstromAngel; // wubrg
};
} // namespace

// =====================================================================================================================
// acceptColor
// =====================================================================================================================

TEST_F(FilterTreeTest, Color)
{
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "wu");
    ASSERT_FALSE(filterTree.acceptsCard(island, language));
    ASSERT_FALSE(filterTree.acceptsCard(artifact, language));
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_TRUE(filterTree.acceptsCard(maelstromAngel, language));
}

TEST_F(FilterTreeTest, ColorColorless)
{
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "c");
    ASSERT_TRUE(filterTree.acceptsCard(island, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_FALSE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_FALSE(filterTree.acceptsCard(maelstromAngel, language));
}

TEST_F(FilterTreeTest, ColorNot)
{
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAndNot, "b");
    ASSERT_TRUE(filterTree.acceptsCard(island, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    ASSERT_TRUE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_FALSE(filterTree.acceptsCard(maelstromAngel, language));
}

TEST_F(FilterTreeTest, ColorAliases)
{
    FilterTree filterTree;

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "green");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(maelstromAngel, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "grn");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(maelstromAngel, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "blue");
    ASSERT_TRUE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "blu");
    ASSERT_TRUE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "black");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(notDead, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "blk");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(notDead, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "red");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(maelstromAngel, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "white");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "wht");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "colorless");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "colourless");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    filterTree.clear();

    filterTree.termNode(CardFilter::Attr::AttrColor, CardFilter::Type::TypeAnd, "none");
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    filterTree.clear();
}

// =====================================================================================================================
// acceptColorIdentity
// =====================================================================================================================

TEST_F(FilterTreeTest, ColorIdentity)
{
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColorIdentity, CardFilter::Type::TypeAnd, "wu");
    ASSERT_FALSE(filterTree.acceptsCard(island, language));
    ASSERT_FALSE(filterTree.acceptsCard(artifact, language));
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_TRUE(filterTree.acceptsCard(maelstromAngel, language));
}

TEST_F(FilterTreeTest, ColorIdentityColorless)
{
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColorIdentity, CardFilter::Type::TypeAnd, "c");
    ASSERT_FALSE(filterTree.acceptsCard(island, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    ASSERT_FALSE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_FALSE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_FALSE(filterTree.acceptsCard(maelstromAngel, language));
}

TEST_F(FilterTreeTest, ColorIdentityNot)
{
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColorIdentity, CardFilter::Type::TypeAndNot, "b");
    ASSERT_TRUE(filterTree.acceptsCard(island, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    ASSERT_TRUE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_FALSE(filterTree.acceptsCard(maelstromAngel, language));
}

TEST_F(FilterTreeTest, ColorIdentityMultiple)
{
    // Simulate a WU commander.
    FilterTree filterTree;
    filterTree.termNode(CardFilter::Attr::AttrColorIdentity, CardFilter::Type::TypeAndNot, "b");
    filterTree.termNode(CardFilter::Attr::AttrColorIdentity, CardFilter::Type::TypeAndNot, "r");
    filterTree.termNode(CardFilter::Attr::AttrColorIdentity, CardFilter::Type::TypeAndNot, "g");
    ASSERT_TRUE(filterTree.acceptsCard(island, language));
    ASSERT_TRUE(filterTree.acceptsCard(artifact, language));
    ASSERT_TRUE(filterTree.acceptsCard(truth, language));
    ASSERT_FALSE(filterTree.acceptsCard(notDead, language));
    ASSERT_TRUE(filterTree.acceptsCard(wallOfDenial, language));
    ASSERT_FALSE(filterTree.acceptsCard(maelstromAngel, language));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
