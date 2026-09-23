#include "gtest/gtest.h"
#include <QList>
#include <QString>
#include <QStringList>
#include <libcockatrice/utility/report_categories.h>

TEST(ReportCategoriesTest, CanonicalListIsNonEmptyAndStable)
{
    const QStringList categories = ReportCategories::keys();

    ASSERT_FALSE(categories.isEmpty());
    EXPECT_EQ("cheating", categories.at(0));
    EXPECT_EQ("other", categories.last());
}

TEST(ReportCategoriesTest, CanonicalListContainsMergedClientAndServerCategories)
{
    const QStringList expected = {"cheating", "bug_abuse", "harassment", "verbal_abuse",
                                  "hate_speech", "spam", "other"};
    EXPECT_EQ(expected, ReportCategories::keys());
    EXPECT_EQ(7, ReportCategories::keys().size());
}

TEST(ReportCategoriesTest, AllCanonicalKeysAreValid)
{
    for (const QString &category : ReportCategories::keys()) {
        EXPECT_TRUE(ReportCategories::isValid(category)) << category.toStdString();
    }
}

TEST(ReportCategoriesTest, LegacyCategoriesRemainValid)
{
    EXPECT_TRUE(ReportCategories::isValid("bug_abuse"));
    EXPECT_TRUE(ReportCategories::isValid("verbal_abuse"));
}

TEST(ReportCategoriesTest, ValidationIsCaseInsensitive)
{
    EXPECT_TRUE(ReportCategories::isValid("Cheating"));
    EXPECT_TRUE(ReportCategories::isValid("HATE_SPEECH"));
}

TEST(ReportCategoriesTest, UnknownAndEmptyCategoriesAreRejected)
{
    EXPECT_FALSE(ReportCategories::isValid(QString()));
    EXPECT_FALSE(ReportCategories::isValid("hate speech"));
    EXPECT_FALSE(ReportCategories::isValid("griefing"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}