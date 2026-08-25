#include "gtest/gtest.h"
#include <cstring>
#include <libcockatrice/utility/passwordhasher.h>

namespace
{

TEST(PasswordHashTest, RegressionTest)
{
    QString salt = "saltsaltsaltsalt";
    QString password = "password";
    QString expected = "vmKoWv975yf+WT2QCXhW48JNzZ2ghGxdgNvuKLBU0h7s6AQHSG72J6QO4ZswuSeqvBbAXbmgJSRBaSJrgc55WA==";
    QString hash = PasswordHasher::computeHash(password, salt);
    ASSERT_EQ(hash, salt + expected) << "The computed hash value remains the same";
}

TEST(PasswordHashTest, SaltUsesAlphanumericCharset)
{
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const QString salt = PasswordHasher::generateRandomSalt();
    ASSERT_EQ(salt.size(), 16);
    for (const QChar &c : salt) {
        ASSERT_NE(strchr(alphanum, c.toLatin1()), nullptr);
    }
}

TEST(PasswordHashTest, SaltsAreUnique)
{
    const QString salt1 = PasswordHasher::generateRandomSalt();
    const QString salt2 = PasswordHasher::generateRandomSalt();
    ASSERT_NE(salt1, salt2);
}

TEST(PasswordHashTest, TokenHasExpectedLength)
{
    const QString token = PasswordHasher::generateActivationToken();
    ASSERT_EQ(token.size(), 16);
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
