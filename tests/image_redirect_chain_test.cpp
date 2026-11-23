#include "../cockatrice/src/interface/card_picture_loader/redirect_chain.h"

#include <gtest/gtest.h>

namespace
{
using RedirectCache = QHash<QUrl, QPair<QUrl, QDateTime>>;

QUrl url(int index)
{
    return QUrl(QStringLiteral("https://cards.example/%1").arg(index));
}

RedirectCache chain(int hops)
{
    RedirectCache cache;
    for (int index = 0; index < hops; ++index) {
        cache.insert(url(index), {url(index + 1), QDateTime()});
    }
    return cache;
}
} // namespace

TEST(ImageRedirectChain, MissingHopReturnsLastKnownUrlWithoutRemovingMappings)
{
    auto cache = chain(2);
    const auto original = cache;
    EXPECT_EQ(resolveImageRedirects(url(0), cache, 10), url(2));
    EXPECT_EQ(cache, original);
    EXPECT_EQ(resolveImageRedirects(url(9), cache, 10), url(9));
}

TEST(ImageRedirectChain, ExactlyMaxHopsResolvesAndKeepsCache)
{
    auto cache = chain(10);
    const auto original = cache;
    EXPECT_EQ(resolveImageRedirects(url(0), cache, 10), url(10));
    EXPECT_EQ(cache, original);
}

TEST(ImageRedirectChain, CycleReturnsOriginalAndPurgesVisitedMappings)
{
    auto cache = chain(2);
    cache.insert(url(2), {url(1), QDateTime()});
    cache.insert(url(9), {url(10), QDateTime()});
    EXPECT_EQ(resolveImageRedirects(url(0), cache, 10), url(0));
    EXPECT_FALSE(cache.contains(url(0)));
    EXPECT_FALSE(cache.contains(url(1)));
    EXPECT_FALSE(cache.contains(url(2)));
    EXPECT_TRUE(cache.contains(url(9)));
}

TEST(ImageRedirectChain, OverlongChainReturnsOriginalAndPurgesVisitedMappings)
{
    auto cache = chain(12);
    cache.insert(url(20), {url(21), QDateTime()});
    EXPECT_EQ(resolveImageRedirects(url(0), cache, 10), url(0));
    for (int index = 0; index < 10; ++index) {
        EXPECT_FALSE(cache.contains(url(index)));
    }
    EXPECT_TRUE(cache.contains(url(10)));
    EXPECT_TRUE(cache.contains(url(11)));
    EXPECT_TRUE(cache.contains(url(20)));
}

TEST(ImageRedirectChain, EmptyTargetStopsAtItsSource)
{
    auto cache = chain(1);
    cache.insert(url(1), {QUrl(), QDateTime()});
    const auto original = cache;
    EXPECT_EQ(resolveImageRedirects(url(0), cache, 10), url(1));
    EXPECT_EQ(cache, original);
}

TEST(ImageRedirectChain, ZeroHopsDoesNotFollowOrRemoveMappings)
{
    auto cache = chain(1);
    EXPECT_EQ(resolveImageRedirects(url(5), cache, 0), url(5));
    EXPECT_TRUE(cache.contains(url(0)));
    EXPECT_EQ(resolveImageRedirects(url(0), cache, 0), url(0));
    EXPECT_TRUE(cache.contains(url(0)));
}
