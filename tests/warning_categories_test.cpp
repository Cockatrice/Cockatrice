#include "gtest/gtest.h"
#include <QString>
#include <libcockatrice/utility/warning_categories.h>

TEST(WarningCategoriesTest, EmptyValueYieldsNoCategories)
{
    EXPECT_TRUE(parseWarningCategories(QString()).isEmpty());
    EXPECT_TRUE(parseWarningCategories(QString("")).isEmpty());
}

TEST(WarningCategoriesTest, PlainNamesDefaultToInterventionLevelOne)
{
    const QList<WarningCategory> categories = parseWarningCategories("Flaming,Spamming,Causing Drama");

    ASSERT_EQ(3, categories.size());
    EXPECT_EQ("Flaming", categories.at(0).name);
    EXPECT_EQ(1, categories.at(0).startingIl);
    EXPECT_EQ("Spamming", categories.at(1).name);
    EXPECT_EQ(1, categories.at(1).startingIl);
    EXPECT_EQ("Causing Drama", categories.at(2).name);
    EXPECT_EQ(1, categories.at(2).startingIl);
}

TEST(WarningCategoriesTest, ExplicitInterventionLevelsAreParsed)
{
    const QList<WarningCategory> categories = parseWarningCategories("Cheating|2,Inappropriate Avatar|3");

    ASSERT_EQ(2, categories.size());
    EXPECT_EQ("Cheating", categories.at(0).name);
    EXPECT_EQ(2, categories.at(0).startingIl);
    EXPECT_EQ("Inappropriate Avatar", categories.at(1).name);
    EXPECT_EQ(3, categories.at(1).startingIl);
}

TEST(WarningCategoriesTest, MixedEntriesKeepDefaultsForThoseWithoutLevels)
{
    const QList<WarningCategory> categories = parseWarningCategories("Abusive Language|1,Cheating|2,Spamming");

    ASSERT_EQ(3, categories.size());
    EXPECT_EQ(1, categories.at(0).startingIl);
    EXPECT_EQ(2, categories.at(1).startingIl);
    EXPECT_EQ("Spamming", categories.at(2).name);
    EXPECT_EQ(1, categories.at(2).startingIl);
}

TEST(WarningCategoriesTest, EmptyEntriesAreSkipped)
{
    const QList<WarningCategory> categories = parseWarningCategories("Spamming,,Cheating|2,");

    ASSERT_EQ(2, categories.size());
    EXPECT_EQ("Spamming", categories.at(0).name);
    EXPECT_EQ("Cheating", categories.at(1).name);
}

TEST(WarningCategoriesTest, WhitespaceIsTrimmed)
{
    const QList<WarningCategory> categories = parseWarningCategories("  Abusive Language , Cheating | 2 ");

    ASSERT_EQ(2, categories.size());
    EXPECT_EQ("Abusive Language", categories.at(0).name);
    EXPECT_EQ(1, categories.at(0).startingIl);
    EXPECT_EQ("Cheating", categories.at(1).name);
    EXPECT_EQ(2, categories.at(1).startingIl);
}

TEST(WarningCategoriesTest, InvalidInterventionLevelsFallBackToOne)
{
    const QList<WarningCategory> categories = parseWarningCategories("Spamming|abc,Cheating|0,Targeted Harassment|-3");

    ASSERT_EQ(3, categories.size());
    EXPECT_EQ(1, categories.at(0).startingIl);
    EXPECT_EQ(1, categories.at(1).startingIl);
    EXPECT_EQ(1, categories.at(2).startingIl);
}

TEST(WarningCategoriesTest, EntryWithOnlyLevelIsSkipped)
{
    const QList<WarningCategory> categories = parseWarningCategories("|2,Spamming|2");

    ASSERT_EQ(1, categories.size());
    EXPECT_EQ("Spamming", categories.at(0).name);
    EXPECT_EQ(2, categories.at(0).startingIl);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
