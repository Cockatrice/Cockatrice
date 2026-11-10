#include "gtest/gtest.h"
#include <QString>
#include <deck_sync_profile.h>

TEST(DeckSyncProfileTest, KeyIsStable)
{
    DeckSyncProfile first("woogerworks.com", 4747, "Skog7");
    DeckSyncProfile second("woogerworks.com", 4747, "Skog7");
    EXPECT_EQ(first.key(), second.key());
    EXPECT_FALSE(first.key().isEmpty());
}

TEST(DeckSyncProfileTest, KeyIgnoresCase)
{
    DeckSyncProfile lower("woogerworks.com", 4747, "skog7");
    DeckSyncProfile upper("WOOGERWORKS.COM", 4747, "SKOG7");
    EXPECT_EQ(lower.key(), upper.key());
}

TEST(DeckSyncProfileTest, KeyDiffersPerServerAndUser)
{
    DeckSyncProfile base("woogerworks.com", 4747, "skog7");
    EXPECT_NE(base.key(), DeckSyncProfile("other.server", 4747, "skog7").key());
    EXPECT_NE(base.key(), DeckSyncProfile("woogerworks.com", 4748, "skog7").key());
    EXPECT_NE(base.key(), DeckSyncProfile("woogerworks.com", 4747, "other").key());
}

TEST(DeckSyncProfileTest, KeyIsUnambiguousAcrossSeparators)
{
    DeckSyncProfile left("a:1", 2, "b");
    DeckSyncProfile right("a", 1, "2:b");
    EXPECT_NE(left.key(), right.key());
    EXPECT_NE(DeckSyncProfile("::1", 4747, "skog7").key(), DeckSyncProfile("::2", 4747, "skog7").key());
}

TEST(DeckSyncProfileTest, InvalidProfilesHaveNoKey)
{
    EXPECT_TRUE(DeckSyncProfile().key().isEmpty());
    EXPECT_TRUE(DeckSyncProfile("", 4747, "skog7").key().isEmpty());
    EXPECT_TRUE(DeckSyncProfile("woogerworks.com", 0, "skog7").key().isEmpty());
}

TEST(DeckSyncProfileTest, KeyIgnoresSurroundingWhitespace)
{
    DeckSyncProfile plain("woogerworks.com", 4747, "skog7");
    DeckSyncProfile padded("  woogerworks.com ", 4747, " skog7 ");
    EXPECT_EQ(plain.key(), padded.key());
}

TEST(DeckSyncProfileTest, ValidityRequiresAllFields)
{
    EXPECT_TRUE(DeckSyncProfile("woogerworks.com", 4747, "skog7").isValid());
    EXPECT_FALSE(DeckSyncProfile("", 4747, "skog7").isValid());
    EXPECT_FALSE(DeckSyncProfile("woogerworks.com", 0, "skog7").isValid());
    EXPECT_FALSE(DeckSyncProfile("woogerworks.com", 4747, "").isValid());
    EXPECT_FALSE(DeckSyncProfile().isValid());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
