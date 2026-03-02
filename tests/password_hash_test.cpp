#include "gtest/gtest.h"
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/rng/rng_sfmt.h>
#include <libcockatrice/utility/passwordhasher.h>

RNG_Abstract *rng;

namespace
{
class PasswordHashTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        rng = new RNG_SFMT;
    }

    void TearDown() override
    {
        delete rng;
    }
};

TEST(PasswordHashTest, RegressionTest)
{
    QString salt = "saltsaltsaltsalt";
    QString password = "password";
    QString expected = "vmKoWv975yf+WT2QCXhW48JNzZ2ghGxdgNvuKLBU0h7s6AQHSG72J6QO4ZswuSeqvBbAXbmgJSRBaSJrgc55WA==";
    QString hash = PasswordHasher::computeHash(password, salt);
    ASSERT_EQ(hash, salt + expected) << "The computed hash value remains the same";
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
