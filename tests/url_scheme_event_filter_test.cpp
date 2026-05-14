#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QFileOpenEvent>
#include <QUrl>
#include <libcockatrice/utility_gui/url_scheme_event_filter.h>

TEST(UrlSchemeEventFilterTest, EmitsAndConsumesMatchingUrl)
{
    UrlSchemeEventFilter filter(QStringLiteral("cockatrice://"));

    int callCount = 0;
    QString received;
    QObject::connect(&filter, &UrlSchemeEventFilter::urlReceived, [&](const QString &url) {
        ++callCount;
        received = url;
    });

    const QString url = QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748");
    QUrl qurl{url};
    QFileOpenEvent event{qurl};
    const bool consumed = filter.eventFilter(nullptr, &event);

    ASSERT_TRUE(consumed) << "Matching URL should be consumed by the filter";
    ASSERT_EQ(callCount, 1) << "urlReceived should have been emitted once";
    ASSERT_EQ(received, url) << "Emitted URL should match the event URL";
}

TEST(UrlSchemeEventFilterTest, PassesThroughNonMatchingUrl)
{
    UrlSchemeEventFilter filter(QStringLiteral("cockatrice://"));

    int callCount = 0;
    QObject::connect(&filter, &UrlSchemeEventFilter::urlReceived, [&](const QString &) { ++callCount; });

    QUrl qurl{QStringLiteral("https://example.com")};
    QFileOpenEvent event{qurl};
    const bool consumed = filter.eventFilter(nullptr, &event);

    ASSERT_FALSE(consumed) << "Non-matching URL should not be consumed";
    ASSERT_EQ(callCount, 0) << "urlReceived should not have been emitted";
}

TEST(UrlSchemeEventFilterTest, MatchesCaseInsensitively)
{
    UrlSchemeEventFilter filter(QStringLiteral("cockatrice://"));

    int callCount = 0;
    QObject::connect(&filter, &UrlSchemeEventFilter::urlReceived, [&](const QString &) { ++callCount; });

    QUrl qurl{QStringLiteral("Cockatrice://joingame?hostname=example.com&port=4748")};
    QFileOpenEvent event{qurl};
    const bool consumed = filter.eventFilter(nullptr, &event);

    ASSERT_TRUE(consumed);
    ASSERT_EQ(callCount, 1);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
