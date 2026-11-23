#include "feature_intro.h"

#include <QSet>
#include <QVersionNumber>
#include <gtest/gtest.h>

namespace
{
TEST(FeatureIntroCatalog, IdsAreUniqueAndStable)
{
    QSet<QString> ids;
    for (const FeatureIntro &intro : featureIntroCatalog()) {
        ASSERT_FALSE(intro.id.isEmpty());
        ASSERT_FALSE(ids.contains(intro.id)) << "duplicate id: " << intro.id.toStdString();
        // The id is the settings key; a slash would silently create a subgroup.
        ASSERT_FALSE(intro.id.contains('/')) << "id must not contain '/': " << intro.id.toStdString();
        ids.insert(intro.id);
    }
}

TEST(FeatureIntroCatalog, EveryEntryCarriesItsCopy)
{
    for (const FeatureIntro &intro : featureIntroCatalog()) {
        EXPECT_FALSE(intro.title.isEmpty()) << intro.id.toStdString();
        EXPECT_FALSE(intro.body.isEmpty()) << intro.id.toStdString();
        EXPECT_FALSE(intro.acceptLabel.isEmpty()) << intro.id.toStdString();
        EXPECT_FALSE(intro.declineLabel.isEmpty()) << intro.id.toStdString();
        EXPECT_FALSE(intro.badgeTooltip.isEmpty()) << intro.id.toStdString();
        EXPECT_FALSE(intro.settingsPage.isEmpty()) << intro.id.toStdString();
        EXPECT_FALSE(intro.settingsQuery.isEmpty()) << intro.id.toStdString();
    }
}

TEST(FeatureIntroCatalog, EveryEntryHasAUsableRevisionAndRelease)
{
    for (const FeatureIntro &intro : featureIntroCatalog()) {
        EXPECT_GE(intro.revision, 1) << intro.id.toStdString();
        EXPECT_FALSE(QVersionNumber::fromString(intro.introducedIn).isNull())
            << intro.id.toStdString() << " introducedIn=" << intro.introducedIn.toStdString();
    }
}

TEST(FeatureIntroCatalog, EveryReworkExplainsItsUndo)
{
    for (const FeatureIntro &intro : featureIntroCatalog()) {
        if (intro.shape != FeatureIntroShape::Rework) {
            continue;
        }
        // "Use classic" flips a real behavior, so the user must be told what they now have.
        EXPECT_FALSE(intro.declinedOutcome.isEmpty()) << intro.id.toStdString();
    }
}

TEST(FeatureIntroCatalog, HoverCardPanelEntryMatchesTheShippedRollout)
{
    const FeatureIntro *intro = findFeatureIntro(QStringLiteral("cards-hover-panel"));
    ASSERT_NE(intro, nullptr);
    ASSERT_EQ(intro->shape, FeatureIntroShape::Rework);
    ASSERT_EQ(intro->introducedIn, QStringLiteral("3.2.0"));
}

TEST(FeatureIntroCatalog, UnknownIdIsNotFound)
{
    ASSERT_EQ(findFeatureIntro(QStringLiteral("no-such-feature")), nullptr);
}
} // namespace
