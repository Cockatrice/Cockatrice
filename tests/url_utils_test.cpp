#include "gtest/gtest.h"
#include <QCoreApplication>
#include <libcockatrice/utility/url_utils.h>

// ---------------------------------------------------------------------------
// UrlUtils::findUrlArgument
// ---------------------------------------------------------------------------

TEST(UrlUtilsTest, FindsMatchingArgument)
{
    const QStringList args{QStringLiteral("--debug"),
                           QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748")};
    ASSERT_EQ(UrlUtils::findUrlArgument(args, QStringLiteral("cockatrice://")),
              QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748"));
}

TEST(UrlUtilsTest, ReturnsEmptyStringWhenNoMatch)
{
    const QStringList args{QStringLiteral("--debug"), QStringLiteral("foo"), QStringLiteral("bar")};
    ASSERT_TRUE(UrlUtils::findUrlArgument(args, QStringLiteral("cockatrice://")).isEmpty());
}

TEST(UrlUtilsTest, FindsArgumentCaseInsensitively)
{
    const QStringList args{QStringLiteral("Cockatrice://joingame?hostname=example.com&port=4748")};
    ASSERT_EQ(UrlUtils::findUrlArgument(args, QStringLiteral("cockatrice://")),
              QStringLiteral("Cockatrice://joingame?hostname=example.com&port=4748"));
}

TEST(UrlUtilsTest, ReturnsFirstMatchOnly)
{
    const QStringList args{QStringLiteral("cockatrice://joingame?first=1"),
                           QStringLiteral("cockatrice://joingame?second=2")};
    ASSERT_EQ(UrlUtils::findUrlArgument(args, QStringLiteral("cockatrice://")),
              QStringLiteral("cockatrice://joingame?first=1"));
}

// ---------------------------------------------------------------------------
// UrlUtils::parseOracleUrl
// ---------------------------------------------------------------------------

TEST(ParseOracleUrlTest, RecognisesUpdate)
{
    const auto action = UrlUtils::parseOracleUrl(QStringLiteral("cockatrice-oracle://update"));
    ASSERT_TRUE(action.isUpdate);
    ASSERT_FALSE(action.spoilersOnly);
}

TEST(ParseOracleUrlTest, RecognisesUpdateWithSpoilers)
{
    const auto action = UrlUtils::parseOracleUrl(QStringLiteral("cockatrice-oracle://update?spoilers=1"));
    ASSERT_TRUE(action.isUpdate);
    ASSERT_TRUE(action.spoilersOnly);
}

TEST(ParseOracleUrlTest, IgnoresUnknownHost)
{
    const auto action = UrlUtils::parseOracleUrl(QStringLiteral("cockatrice-oracle://unrelated"));
    ASSERT_FALSE(action.isUpdate);
}

TEST(ParseOracleUrlTest, MatchesHostCaseInsensitively)
{
    const auto action = UrlUtils::parseOracleUrl(QStringLiteral("cockatrice-oracle://UPDATE"));
    ASSERT_TRUE(action.isUpdate);
}

// ---------------------------------------------------------------------------
// UrlUtils::parseJoinGameUrl
// ---------------------------------------------------------------------------

namespace
{
const QString kValidUrl = QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748&roomid=1&gameid=42");
}

TEST(ParseJoinGameUrlTest, ParsesHappyPath)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(kValidUrl);
    ASSERT_TRUE(parsed.has_value());
    ASSERT_EQ(parsed->hostname, QStringLiteral("example.com"));
    ASSERT_EQ(parsed->port, 4748);
    ASSERT_EQ(parsed->roomId, 1);
    ASSERT_EQ(parsed->gameId, 42);
    ASSERT_FALSE(parsed->spectator);
}

TEST(ParseJoinGameUrlTest, AcceptsSpectateFlag)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(kValidUrl + QStringLiteral("&spectate=1"));
    ASSERT_TRUE(parsed.has_value());
    ASSERT_TRUE(parsed->spectator);
}

TEST(ParseJoinGameUrlTest, SpectateZeroIsNotSpectator)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(kValidUrl + QStringLiteral("&spectate=0"));
    ASSERT_TRUE(parsed.has_value());
    ASSERT_FALSE(parsed->spectator);
}

TEST(ParseJoinGameUrlTest, MatchesSchemeCaseInsensitively)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("Cockatrice://joingame?hostname=example.com&port=4748&roomid=1&gameid=42"));
    ASSERT_TRUE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsInvalidScheme)
{
    const auto parsed =
        UrlUtils::parseJoinGameUrl(QStringLiteral("http://joingame?hostname=example.com&port=4748&roomid=1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsUnsupportedHost)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("cockatrice://something?hostname=example.com&port=4748&roomid=1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsMissingHostname)
{
    const auto parsed =
        UrlUtils::parseJoinGameUrl(QStringLiteral("cockatrice://joingame?port=4748&roomid=1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsZeroPort)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("cockatrice://joingame?hostname=example.com&port=0&roomid=1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsOutOfRangePort)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("cockatrice://joingame?hostname=example.com&port=99999&roomid=1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsNonNumericPort)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("cockatrice://joingame?hostname=example.com&port=abc&roomid=1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsNegativeRoomId)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748&roomid=-1&gameid=42"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, RejectsNegativeGameId)
{
    const auto parsed = UrlUtils::parseJoinGameUrl(
        QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748&roomid=1&gameid=-1"));
    ASSERT_FALSE(parsed.has_value());
}

TEST(ParseJoinGameUrlTest, IgnoresCredentialQueryParams)
{
    // Regression test for the security blocker: even if username/password are
    // present in the URL (e.g. legacy bookmark), they must not surface in the
    // parsed output.  Parsing should succeed and yield the same params as the
    // equivalent URL without those fields.
    const auto withCreds = UrlUtils::parseJoinGameUrl(kValidUrl + QStringLiteral("&username=alice&password=hunter2"));
    const auto withoutCreds = UrlUtils::parseJoinGameUrl(kValidUrl);
    ASSERT_TRUE(withCreds.has_value());
    ASSERT_TRUE(withoutCreds.has_value());
    ASSERT_EQ(withCreds->hostname, withoutCreds->hostname);
    ASSERT_EQ(withCreds->port, withoutCreds->port);
    ASSERT_EQ(withCreds->roomId, withoutCreds->roomId);
    ASSERT_EQ(withCreds->gameId, withoutCreds->gameId);
    ASSERT_EQ(withCreds->spectator, withoutCreds->spectator);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
