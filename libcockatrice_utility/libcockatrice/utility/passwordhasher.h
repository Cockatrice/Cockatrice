#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <QByteArray>
#include <QObject>

// scrypt cost parameters used for newly created password verifiers. These match
// the RFC 7914 recommended parameters for interactive use.
constexpr int SCRYPT_N = 32768;
constexpr int SCRYPT_R = 8;
constexpr int SCRYPT_P = 1;
constexpr int SCRYPT_SALT_LENGTH = 16;
constexpr int SCRYPT_VERIFIER_LENGTH = 64;

enum class PasswordFormat
{
    None = 0,
    Scrypt
};

struct PasswordVerifier
{
    PasswordFormat format = PasswordFormat::None;
    int n = 0;
    int r = 0;
    int p = 0;
    QByteArray salt;
    QByteArray verifier;
    bool isValid = false;
};

class PasswordHasher
{
public:
    static QString computeHash(const QString &password, const QString &salt);
    static QString generateRandomSalt(const int len = 16);
    static QString generateActivationToken();

    /** @brief Derive the scrypt verifier for the given password, salt and cost parameters. */
    static QByteArray deriveKey(const QString &password, const QByteArray &salt, int n, int r, int p);
    /** @brief Build a "$scrypt$<n>$<r>$<p>$<salt>$<verifier>" string with a fresh random salt. */
    static QString generatePasswordVerifier(const QString &password);
    /** @brief Parse a stored "$scrypt$..." string into its components. */
    static PasswordVerifier parsePasswordVerifier(const QString &stored);
    /** @brief True if the stored value is not in the scrypt format (legacy salt+hash). */
    static bool isLegacyFormat(const QString &stored);
    /** @brief HMAC-SHA256 of nonce keyed with the password verifier, used for challenge-response logins. */
    static QByteArray computeResponse(const QByteArray &key, const QByteArray &nonce);
    /** @brief Constant-time byte comparison. */
    static bool constantTimeEquals(const QByteArray &a, const QByteArray &b);
};

#endif
