#include "feature_intro.h"
#include "feature_intro_planner.h"

#include <gtest/gtest.h>
#include <optional>

namespace
{
FeatureIntro entry(const QString &id, const QString &introducedIn, int revision = 1)
{
    FeatureIntro intro;
    intro.id = id;
    intro.introducedIn = introducedIn;
    intro.revision = revision;
    return intro;
}

QStringList writtenIds(const FeatureIntroPlan &plan)
{
    QStringList ids;
    ids.reserve(plan.writes.size());
    for (const auto &write : plan.writes) {
        ids.append(write.first);
    }
    return ids;
}

std::optional<FeatureIntroState> writtenState(const FeatureIntroPlan &plan, const QString &id)
{
    for (const auto &write : plan.writes) {
        if (write.first == id) {
            return write.second;
        }
    }
    return std::nullopt;
}

QHash<QString, FeatureIntroState> states()
{
    return {};
}
} // namespace

TEST(FeatureIntroPlanner, CleanInstall_BaselinesEveryEntry)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0"), entry("other", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, QString(), "3.2.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenIds(plan), (QStringList{"cards-hover-panel", "other"}));
    for (const auto &write : plan.writes) {
        ASSERT_EQ(write.second.decision, FeatureIntroDecision::Baseline);
        ASSERT_EQ(write.second.decidedAt, 0);
    }
}

TEST(FeatureIntroPlanner, UpdateFromBeforeFeature_QueuesIt)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "3.1.2", "3.2.0");

    ASSERT_EQ(plan.due, QStringList{"cards-hover-panel"});
    const auto state = writtenState(plan, "cards-hover-panel");
    ASSERT_TRUE(state.has_value());
    ASSERT_EQ(state->decision, FeatureIntroDecision::Pending);
    ASSERT_EQ(state->revision, 1);
}

TEST(FeatureIntroPlanner, UpdateFromAfterFeature_Baselines)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "3.3.0", "3.4.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenState(plan, "cards-hover-panel")->decision, FeatureIntroDecision::Baseline);
}

TEST(FeatureIntroPlanner, RestartOnTheSameVersion_Baselines)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "3.2.0", "3.2.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenState(plan, "cards-hover-panel")->decision, FeatureIntroDecision::Baseline);
}

TEST(FeatureIntroPlanner, ReleaseWithoutTheFeature_Baselines)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.3.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "3.1.0", "3.2.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenState(plan, "cards-hover-panel")->decision, FeatureIntroDecision::Baseline);
}

TEST(FeatureIntroPlanner, PrereleaseSuffixDoesNotHideTheUpdate)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "3.1.2", "3.2.0-beta.3 (2026-11-10)");

    ASSERT_EQ(plan.due, QStringList{"cards-hover-panel"});
}

TEST(FeatureIntroPlanner, UnparsablePreviousVersion_Baselines)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "development", "3.2.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenState(plan, "cards-hover-panel")->decision, FeatureIntroDecision::Baseline);
}

TEST(FeatureIntroPlanner, ExistingDecision_IsLeftAlone)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};
    QHash<QString, FeatureIntroState> known = states();
    known.insert("cards-hover-panel",
                 FeatureIntroState{.decision = FeatureIntroDecision::Accepted, .revision = 1, .decidedAt = 111});

    const FeatureIntroPlan plan = planFeatureIntros(catalog, known, {}, "3.1.0", "3.2.0");

    ASSERT_TRUE(plan.writes.isEmpty());
    ASSERT_TRUE(plan.due.isEmpty());
}

TEST(FeatureIntroPlanner, PendingState_IsDueWithoutAWrite)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};
    QHash<QString, FeatureIntroState> known = states();
    known.insert("cards-hover-panel",
                 FeatureIntroState{.decision = FeatureIntroDecision::Pending, .revision = 1, .decidedAt = 0});

    const FeatureIntroPlan plan = planFeatureIntros(catalog, known, {}, "3.1.0", "3.2.0");

    ASSERT_TRUE(plan.writes.isEmpty());
    ASSERT_EQ(plan.due, QStringList{"cards-hover-panel"});
}

TEST(FeatureIntroPlanner, RevisionBump_RequeuesExactlyOnce)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0", 2)};
    QHash<QString, FeatureIntroState> known = states();
    known.insert("cards-hover-panel",
                 FeatureIntroState{.decision = FeatureIntroDecision::Declined, .revision = 1, .decidedAt = 111});

    const FeatureIntroPlan first = planFeatureIntros(catalog, known, {}, "3.1.0", "3.2.0");

    ASSERT_EQ(writtenIds(first), QStringList{"cards-hover-panel"});
    const auto requeued = writtenState(first, "cards-hover-panel");
    ASSERT_EQ(requeued->decision, FeatureIntroDecision::Pending);
    ASSERT_EQ(requeued->revision, 2);
    ASSERT_EQ(requeued->decidedAt, 0);
    ASSERT_EQ(first.due, QStringList{"cards-hover-panel"});

    // The write above is what stops the next launch from queueing again.
    known.insert("cards-hover-panel", *requeued);
    const FeatureIntroPlan second = planFeatureIntros(catalog, known, {}, "3.1.0", "3.2.0");

    ASSERT_TRUE(second.writes.isEmpty());
    ASSERT_EQ(second.due, QStringList{"cards-hover-panel"});
}

TEST(FeatureIntroPlanner, ConfiguredFeature_IsSkipped)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {"cards-hover-panel"}, "3.1.0", "3.2.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenState(plan, "cards-hover-panel")->decision, FeatureIntroDecision::Skipped);
}

TEST(FeatureIntroPlanner, ConfiguredFeatureAtRevisionBump_IsSkipped)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0", 2)};
    QHash<QString, FeatureIntroState> known = states();
    known.insert("cards-hover-panel",
                 FeatureIntroState{.decision = FeatureIntroDecision::Declined, .revision = 1, .decidedAt = 111});

    const FeatureIntroPlan plan = planFeatureIntros(catalog, known, {"cards-hover-panel"}, "3.1.0", "3.2.0");

    ASSERT_TRUE(plan.due.isEmpty());
    ASSERT_EQ(writtenState(plan, "cards-hover-panel")->decision, FeatureIntroDecision::Skipped);
}

TEST(FeatureIntroPlanner, OrderFollowsTheCatalog)
{
    const QVector<FeatureIntro> catalog = {entry("first", "3.2.0"), entry("second", "3.2.0")};

    const FeatureIntroPlan plan = planFeatureIntros(catalog, states(), {}, "3.1.0", "3.2.0");

    ASSERT_EQ(plan.due, (QStringList{"first", "second"}));
}

TEST(FeatureIntroPlanner, RetiredEntry_KeepsItsStoredState)
{
    const QVector<FeatureIntro> catalog = {entry("cards-hover-panel", "3.2.0")};
    QHash<QString, FeatureIntroState> known = states();
    known.insert("retired", FeatureIntroState{.decision = FeatureIntroDecision::Accepted, .revision = 1});

    const FeatureIntroPlan plan = planFeatureIntros(catalog, known, {}, "3.1.0", "3.2.0");

    ASSERT_EQ(plan.writes.size(), 1);
    ASSERT_EQ(plan.writes.first().first, QStringLiteral("cards-hover-panel"));
}
