#include "gtest/gtest.h"
#include <QString>
#include <libcockatrice/deck_list/deck_list.h>

namespace
{
QString deckWithName(const QString &name)
{
    return QStringLiteral(R"(<?xml version="1.0"?><cockatrice_deck version="1">)"
                          R"(<deckname>%1</deckname><comments></comments>)"
                          R"(<zone name="main"><card number="4" name="Lightning Bolt"/></zone>)"
                          R"(<zone name="side"><card number="2" name="Pyroblast"/></zone>)"
                          R"(</cockatrice_deck>)")
        .arg(name);
}
} // namespace

TEST(DeckContentHashTest, MatchesNativeSerialisation)
{
    DeckList deck;
    ASSERT_TRUE(deck.loadFromString_Native(deckWithName(QStringLiteral("Burn"))));
    EXPECT_EQ(deck.getContentHash(), DeckList::contentHash(deck.writeToString_Native()));
}

TEST(DeckContentHashTest, IsStableAcrossLoads)
{
    DeckList first;
    DeckList second;
    ASSERT_TRUE(first.loadFromString_Native(deckWithName(QStringLiteral("Burn"))));
    ASSERT_TRUE(second.loadFromString_Native(deckWithName(QStringLiteral("Burn"))));
    EXPECT_EQ(first.getContentHash(), second.getContentHash());
}

TEST(DeckContentHashTest, CoversDeckName)
{
    DeckList burn;
    DeckList renamed;
    ASSERT_TRUE(burn.loadFromString_Native(deckWithName(QStringLiteral("Burn"))));
    ASSERT_TRUE(renamed.loadFromString_Native(deckWithName(QStringLiteral("Burn v2"))));

    // The card hash only looks at card names and counts, so it cannot tell the
    // two decks apart; the content hash must.
    EXPECT_EQ(burn.getDeckHash(), renamed.getDeckHash());
    EXPECT_NE(burn.getContentHash(), renamed.getContentHash());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
