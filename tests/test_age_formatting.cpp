#include "../cockatrice/src/interface/widgets/server/user/user_info_box.h"

#include "gtest/gtest.h"

namespace
{
using dayyear = QPair<int, int>;

TEST(AgeFormatting, Zero)
{
    auto got = UserInfoBox::getDaysAndYearsBetween(QDate(2000, 1, 1), QDate(2000, 1, 1));
    ASSERT_EQ(got, dayyear(0, 0)) << "these are the same day";
}

TEST(AgeFormatting, LeapDay)
{
    auto got = UserInfoBox::getDaysAndYearsBetween(QDate(2000, 2, 28), QDate(2000, 3, 1));
    ASSERT_EQ(got, dayyear(2, 0)) << "there is a leap day in between these days";
}

TEST(AgeFormatting, LeapYear)
{
    auto got = UserInfoBox::getDaysAndYearsBetween(QDate(2000, 1, 1), QDate(2001, 1, 1));
    ASSERT_EQ(got, dayyear(0, 1)) << "there is a leap day in between these dates, but that's fine";
}

TEST(AgeFormatting, LeapDayWithYear)
{
    auto got = UserInfoBox::getDaysAndYearsBetween(QDate(2000, 2, 28), QDate(2001, 3, 1));
    ASSERT_EQ(got, dayyear(1, 1)) << "there is a leap day in between these days but not in the last year";
}

TEST(AgeFormatting, LeapDayThisYear)
{
    auto got = UserInfoBox::getDaysAndYearsBetween(QDate(2003, 2, 28), QDate(2004, 3, 1));
    ASSERT_EQ(got, dayyear(2, 1)) << "there is a leap day in between these days this year";
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
