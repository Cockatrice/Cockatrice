#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/playmat_resolver.h>

namespace
{

PlaymatResolution makeResolution(const QString &name, const QString &providerId = QString())
{
    PlaymatResolution resolution;
    resolution.card = {name, providerId};
    return resolution;
}

} // namespace

TEST(PlaymatResolverTest, EmptyChainReturnsEmpty)
{
    DeckList deck;
    const PlaymatResolution resolved = resolveEffectivePlaymat(deck, {}, {}, PlaymatFallbackMode::Fixed, 0);
    EXPECT_TRUE(resolved.card.isEmpty());
}

TEST(PlaymatResolverTest, OverrideWinsOverDeckAndFallback)
{
    DeckList deck;
    deck.setPlaymat({{QStringLiteral("Deck Mat"), QStringLiteral("deck-provider")}, {}});

    const PlaymatResolution force = makeResolution(QStringLiteral("Force Mat"), QStringLiteral("force-provider"));
    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("Fallback Mat"))};

    const PlaymatResolution resolved = resolveEffectivePlaymat(deck, force, fallback, PlaymatFallbackMode::Fixed, 0);
    EXPECT_EQ(resolved.card.name, QStringLiteral("Force Mat"));
    EXPECT_EQ(resolved.card.providerId, QStringLiteral("force-provider"));
}

TEST(PlaymatResolverTest, DeckWinsOverFallback)
{
    DeckList deck;
    deck.setPlaymat({{QStringLiteral("Deck Mat"), QStringLiteral("deck-provider")}, {0.1, 0.2, 0.3, 1.5}});

    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("Fallback Mat"))};

    const PlaymatResolution resolved = resolveEffectivePlaymat(deck, {}, fallback, PlaymatFallbackMode::Fixed, 0);
    EXPECT_EQ(resolved.card.name, QStringLiteral("Deck Mat"));
    EXPECT_EQ(resolved.card.providerId, QStringLiteral("deck-provider"));
    EXPECT_DOUBLE_EQ(resolved.params.marginPctL, 0.1);
    EXPECT_DOUBLE_EQ(resolved.params.verticalOffset, 0.3);
}

TEST(PlaymatResolverTest, FallbackUsedWhenDeckHasNone)
{
    DeckList deck;
    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("Fallback Mat"))};

    const PlaymatResolution resolved = resolveEffectivePlaymat(deck, {}, fallback, PlaymatFallbackMode::Fixed, 0);
    EXPECT_EQ(resolved.card.name, QStringLiteral("Fallback Mat"));
}

TEST(PlaymatResolverTest, FixedAlwaysUsesFirst)
{
    DeckList deck;
    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("First")),
                                               makeResolution(QStringLiteral("Second"))};

    for (int i = 0; i < 5; ++i) {
        const PlaymatResolution resolved = resolveEffectivePlaymat(deck, {}, fallback, PlaymatFallbackMode::Fixed, i);
        EXPECT_EQ(resolved.card.name, QStringLiteral("First"));
    }
}

TEST(PlaymatResolverTest, RoundRobinCyclesAndWraps)
{
    DeckList deck;
    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("First")),
                                               makeResolution(QStringLiteral("Second")),
                                               makeResolution(QStringLiteral("Third"))};

    const QStringList expected = {QStringLiteral("First"), QStringLiteral("Second"), QStringLiteral("Third"),
                                  QStringLiteral("First"), QStringLiteral("Second"), QStringLiteral("Third")};
    for (int i = 0; i < expected.size(); ++i) {
        const PlaymatResolution resolved =
            resolveEffectivePlaymat(deck, {}, fallback, PlaymatFallbackMode::RoundRobin, i);
        EXPECT_EQ(resolved.card.name, expected.at(i));
    }
}

TEST(PlaymatResolverTest, RoundRobinRespectsCursor)
{
    DeckList deck;
    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("First")),
                                               makeResolution(QStringLiteral("Second"))};

    const PlaymatResolution resolved = resolveEffectivePlaymat(deck, {}, fallback, PlaymatFallbackMode::RoundRobin, 5);
    EXPECT_EQ(resolved.card.name, QStringLiteral("Second")); // 5 % 2 == 1
}

TEST(PlaymatResolverTest, RandomStaysWithinList)
{
    DeckList deck;
    const QList<PlaymatResolution> fallback = {makeResolution(QStringLiteral("First")),
                                               makeResolution(QStringLiteral("Second")),
                                               makeResolution(QStringLiteral("Third"))};

    for (int i = 0; i < 50; ++i) {
        const PlaymatResolution resolved = resolveEffectivePlaymat(deck, {}, fallback, PlaymatFallbackMode::Random, i);
        ASSERT_FALSE(resolved.card.name.isEmpty());
        EXPECT_TRUE(fallback.contains(resolved));
    }
}

TEST(PlaymatResolverTest, ForceWithEmptyCardIgnoresFallbackParamsButNotFallback)
{
    DeckList deck;
    deck.setPlaymat({{QStringLiteral("Deck Mat")}, {}});

    // An empty force entry must not mask the deck-configured playmat.
    const PlaymatResolution emptyForce;
    const PlaymatResolution resolved = resolveEffectivePlaymat(deck, emptyForce, {}, PlaymatFallbackMode::Fixed, 0);
    EXPECT_EQ(resolved.card.name, QStringLiteral("Deck Mat"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
