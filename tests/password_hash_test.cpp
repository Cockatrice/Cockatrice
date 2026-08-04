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

TEST(PasswordHashTest, DeriveKeyMatchesKnownVector)
{
    // RFC 7914 scrypt test vector, P="password", S="NaCl", N=1024, r=8, p=16
    const QByteArray expected = QByteArray::fromHex("fdbabe1c9d3472007856e7190d01e9fe7c6ad7cbc8237830e77376634b"
                                                    "3731622eaf30d92e22a3886ff109279d9830dac727afb94a83ee6d8360cb"
                                                    "dfa2cc0640");
    const QByteArray derived = PasswordHasher::deriveKey("password", QByteArray("NaCl"), 1024, 8, 16);
    ASSERT_EQ(derived.toHex(), expected.toHex());
}

TEST(PasswordHashTest, PasswordVerifierRoundTrip)
{
    const QString stored = PasswordHasher::generatePasswordVerifier("hunter2");
    ASSERT_FALSE(stored.isEmpty());
    ASSERT_TRUE(stored.startsWith("$scrypt$"));

    const PasswordVerifier parsed = PasswordHasher::parsePasswordVerifier(stored);
    ASSERT_TRUE(parsed.isValid);
    ASSERT_EQ(parsed.format, PasswordFormat::Scrypt);
    ASSERT_EQ(parsed.n, SCRYPT_N);
    ASSERT_EQ(parsed.r, SCRYPT_R);
    ASSERT_EQ(parsed.p, SCRYPT_P);
    ASSERT_EQ(parsed.salt.size(), SCRYPT_SALT_LENGTH);
    ASSERT_EQ(parsed.verifier.size(), SCRYPT_VERIFIER_LENGTH);
}

TEST(PasswordHashTest, PasswordVerifierInvalidInput)
{
    ASSERT_FALSE(PasswordHasher::parsePasswordVerifier("garbage").isValid);
    ASSERT_FALSE(PasswordHasher::parsePasswordVerifier("$scrypt$not-an-int$8$1$AAAA$BBBB").isValid);
    ASSERT_FALSE(PasswordHasher::parsePasswordVerifier("$scrypt$1024$8$1$AAAA$too-short").isValid);
    ASSERT_FALSE(PasswordHasher::parsePasswordVerifier("$pbkdf2-sha512$1000$AAAA$BBBB").isValid);
}

TEST(PasswordHashTest, LegacyFormatDetection)
{
    ASSERT_TRUE(PasswordHasher::isLegacyFormat("salt+hash"));
    ASSERT_FALSE(PasswordHasher::isLegacyFormat(PasswordHasher::generatePasswordVerifier("password")));
}

TEST(PasswordHashTest, DeriveKeyDependsOnCostParameters)
{
    const QByteArray keyA = PasswordHasher::deriveKey("password", QByteArray("NaCl"), 1024, 8, 16);
    const QByteArray keyB = PasswordHasher::deriveKey("password", QByteArray("NaCl"), 2048, 8, 16);
    const QByteArray keyC = PasswordHasher::deriveKey("password", QByteArray("NaCl"), 1024, 8, 1);
    ASSERT_NE(keyA, keyB);
    ASSERT_NE(keyA, keyC);
}

TEST(PasswordHashTest, ComputeResponseIsDeterministic)
{
    const QByteArray nonce = QByteArray("a nonce value");
    const QByteArray key = QByteArray("the verifier bytes");
    const QByteArray r1 = PasswordHasher::computeResponse(key, nonce);
    const QByteArray r2 = PasswordHasher::computeResponse(key, nonce);
    const QByteArray r3 = PasswordHasher::computeResponse(QByteArray("a different key"), nonce);
    ASSERT_EQ(r1, r2);
    ASSERT_NE(r1, r3);
}

TEST(PasswordHashTest, ConstantTimeEquals)
{
    ASSERT_TRUE(PasswordHasher::constantTimeEquals(QByteArray("same"), QByteArray("same")));
    ASSERT_FALSE(PasswordHasher::constantTimeEquals(QByteArray("same"), QByteArray("diff")));
    ASSERT_FALSE(PasswordHasher::constantTimeEquals(QByteArray("short"), QByteArray("longer")));
}

TEST(PasswordHashTest, CostParamsAreSane)
{
    // Accept the recommended interactive parameters and the RFC 7914 test vector's.
    ASSERT_TRUE(PasswordHasher::costParamsAreSane(SCRYPT_N, SCRYPT_R, SCRYPT_P));
    ASSERT_TRUE(PasswordHasher::costParamsAreSane(1024, 8, 16));

    // n must be in [1024, 2**20] and a power of two.
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(512, 8, 1));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(1 << 21, 8, 1));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(1025, 8, 1));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(0, 8, 1));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(-1024, 8, 1));

    // r in [1, 32], p in [1, 16].
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(1024, 0, 1));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(1024, 33, 1));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(1024, 8, 0));
    ASSERT_FALSE(PasswordHasher::costParamsAreSane(1024, 8, 17));
}

TEST(PasswordHashTest, ParsePasswordVerifierRejectsHostileCostParams)
{
    // 16 bytes of salt and 64 bytes of verifier, base64 encoded.
    const QString saltB64 = QLatin1String("c2FsdHNhbHRzYWx0c2FsdA==");
    const QString verifierB64 = QString(QByteArray(SCRYPT_VERIFIER_LENGTH, '\x42').toBase64());

    ASSERT_TRUE(
        PasswordHasher::parsePasswordVerifier(QString("$scrypt$1024$8$1$%1$%2").arg(saltB64).arg(verifierB64)).isValid);

    // n not a power of two, below 1024, or above 2**20.
    ASSERT_FALSE(
        PasswordHasher::parsePasswordVerifier(QString("$scrypt$1025$8$1$%1$%2").arg(saltB64).arg(verifierB64)).isValid);
    ASSERT_FALSE(
        PasswordHasher::parsePasswordVerifier(QString("$scrypt$512$8$1$%1$%2").arg(saltB64).arg(verifierB64)).isValid);
    ASSERT_FALSE(
        PasswordHasher::parsePasswordVerifier(QString("$scrypt$1073741824$8$1$%1$%2").arg(saltB64).arg(verifierB64))
            .isValid);

    // r and p out of range.
    ASSERT_FALSE(PasswordHasher::parsePasswordVerifier(QString("$scrypt$1024$33$1$%1$%2").arg(saltB64).arg(verifierB64))
                     .isValid);
    ASSERT_FALSE(PasswordHasher::parsePasswordVerifier(QString("$scrypt$1024$8$17$%1$%2").arg(saltB64).arg(verifierB64))
                     .isValid);
}

TEST(PasswordHashTest, VerifyPasswordLegacyRow)
{
    const QString salt = PasswordHasher::generateRandomSalt();
    const QString legacyStored = PasswordHasher::computeHash("correct horse", salt);
    ASSERT_TRUE(PasswordHasher::verifyPassword("correct horse", legacyStored));
    ASSERT_FALSE(PasswordHasher::verifyPassword("battery staple", legacyStored));
}

TEST(PasswordHashTest, VerifyPasswordScryptRow)
{
    const QString scryptStored = PasswordHasher::generatePasswordVerifier("correct horse");
    // Regression for the changeUserPassword bug that re-hashed the old password with
    // a 16-char salt torn out of the "$scrypt$..." string, which could never match.
    ASSERT_TRUE(PasswordHasher::verifyPassword("correct horse", scryptStored));
    ASSERT_FALSE(PasswordHasher::verifyPassword("battery staple", scryptStored));
    ASSERT_FALSE(PasswordHasher::verifyPassword("correct horse", "garbage"));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
