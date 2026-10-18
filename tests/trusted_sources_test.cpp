#include "trusted_sources.h"

#include "gtest/gtest.h"
#include <QHostAddress>

namespace
{

const QString kDefault = QStringLiteral("127.0.0.1,::1");

TEST(TrustedSourcesTest, MatchesListedAddresses)
{
    ASSERT_TRUE(isTrustedSource(kDefault, QHostAddress("127.0.0.1")));
    ASSERT_TRUE(isTrustedSource(kDefault, QHostAddress("::1")));
    ASSERT_FALSE(isTrustedSource(kDefault, QHostAddress("10.0.0.1")));
}

TEST(TrustedSourcesTest, MatchesIpv4MappedPeerAgainstPlainIpv4Entry)
{
    // A dual-stack listener reports a local IPv4 proxy as ::ffff:127.0.0.1;
    // it must still match the default 127.0.0.1 entry.
    ASSERT_TRUE(isTrustedSource(kDefault, QHostAddress("::ffff:127.0.0.1")));
}

TEST(TrustedSourcesTest, MatchesPlainIpv4PeerAgainstMappedEntry)
{
    ASSERT_TRUE(isTrustedSource("::ffff:192.0.2.7", QHostAddress("192.0.2.7")));
}

TEST(TrustedSourcesTest, DoesNotMatchSubstrings)
{
    // Substring matching would let 27.0.0.1 or 0.0.1 pass as 127.0.0.1.
    ASSERT_FALSE(isTrustedSource(kDefault, QHostAddress("27.0.0.1")));
    ASSERT_FALSE(isTrustedSource("192.0.2.10", QHostAddress("192.0.2.1")));
    ASSERT_FALSE(isTrustedSource(kDefault, QHostAddress(":")));
}

TEST(TrustedSourcesTest, ToleratesWhitespaceAndCase)
{
    ASSERT_TRUE(isTrustedSource(" 192.0.2.7 , 2001:DB8::1 ", QHostAddress("2001:db8::1")));
    ASSERT_TRUE(isTrustedSource(" 192.0.2.7 , 2001:DB8::1 ", QHostAddress("192.0.2.7")));
}

TEST(TrustedSourcesTest, IgnoresMalformedEntriesAndNullAddresses)
{
    ASSERT_FALSE(isTrustedSource("not-an-ip,,", QHostAddress("127.0.0.1")));
    ASSERT_FALSE(isTrustedSource("", QHostAddress("127.0.0.1")));
    ASSERT_FALSE(isTrustedSource(kDefault, QHostAddress()));
}

TEST(TrustedSourcesTest, StringOverloadParsesAddress)
{
    ASSERT_TRUE(isTrustedSource(kDefault, QStringLiteral("::ffff:127.0.0.1")));
    ASSERT_FALSE(isTrustedSource(kDefault, QStringLiteral("27.0.0.1")));
    ASSERT_FALSE(isTrustedSource(kDefault, QString()));
}

} // namespace
