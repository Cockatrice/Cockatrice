#include "forwarded_address.h"

#include "gtest/gtest.h"

namespace
{

TEST(ForwardedAddressTest, ParsesSingleAddress)
{
    EXPECT_EQ(QHostAddress("203.0.113.7"), parseForwardedAddress("203.0.113.7"));
    EXPECT_EQ(QHostAddress("::1"), parseForwardedAddress("::1"));
    EXPECT_EQ(QHostAddress("203.0.113.7"), parseForwardedAddress("  203.0.113.7  "));
}

TEST(ForwardedAddressTest, UsesRightmostEntryOfAList)
{
    // nginx's $proxy_add_x_forwarded_for appends its own view of the client to
    // whatever the client sent, so only the trailing entry is trustworthy.
    EXPECT_EQ(QHostAddress("203.0.113.7"), parseForwardedAddress("198.51.100.9, 203.0.113.7"));
    EXPECT_EQ(QHostAddress("203.0.113.7"), parseForwardedAddress("spoofed.invalid, 203.0.113.7"));
    EXPECT_EQ(QHostAddress("203.0.113.7"), parseForwardedAddress("203.0.113.7, "));
}

TEST(ForwardedAddressTest, KeepsIpv4MappedEntries)
{
    EXPECT_EQ(QHostAddress("::ffff:203.0.113.7"), parseForwardedAddress("::ffff:203.0.113.7"));
}

TEST(ForwardedAddressTest, RejectsValuesWithoutAnAddress)
{
    EXPECT_TRUE(parseForwardedAddress("").isNull());
    EXPECT_TRUE(parseForwardedAddress(",").isNull());
    EXPECT_TRUE(parseForwardedAddress("unknown").isNull());
    EXPECT_TRUE(parseForwardedAddress("garbage, more garbage").isNull());
}

} // namespace
