#include "passwordhasher.h"

#include <QCryptographicHash>
#include <libcockatrice/utility/cryptoutil.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

QString PasswordHasher::computeHash(const QString &password, const QString &salt)
{
    QCryptographicHash::Algorithm algo = QCryptographicHash::Sha512;
    const int rounds = 1000;

    QByteArray hash = (salt + password).toUtf8();
    for (int i = 0; i < rounds; ++i) {
        hash = QCryptographicHash::hash(hash, algo);
    }
    QString hashedPass = salt + QString(hash.toBase64());
    return hashedPass;
}

QString PasswordHasher::generateRandomSalt(const int len)
{
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";
    const int size = sizeof(alphanum) - 1;

    // Two bytes per character, corrected for modulo bias via rejection sampling.
    const int bucketSize = 65536 / size;
    const int limit = bucketSize * size;

    QString ret;
    ret.reserve(len);
    QByteArray random = CryptoUtil::randomBytes(len * 2);
    int bytesUsed = 0;
    for (int i = 0; i < len; ++i) {
        unsigned int value;
        do {
            if (bytesUsed >= random.size()) {
                random = CryptoUtil::randomBytes(len * 2);
                bytesUsed = 0;
            }
            value = static_cast<unsigned int>(static_cast<unsigned char>(random.at(bytesUsed))) << 8 |
                    static_cast<unsigned int>(static_cast<unsigned char>(random.at(bytesUsed + 1)));
            bytesUsed += 2;
        } while (value >= limit);
        ret.append(alphanum[value / bucketSize]);
    }

    return ret;
}

QString PasswordHasher::generateActivationToken()
{
    return QString(CryptoUtil::randomBytes(16).toBase64().left(16));
}

QByteArray PasswordHasher::deriveKey(const QString &password, const QByteArray &salt, int n, int r, int p)
{
    QByteArray key(SCRYPT_VERIFIER_LENGTH, '\0');
    const QByteArray passwordUtf8 = password.toUtf8();
    // EVP_PBE_scrypt aborts unless maxmem covers the required working memory,
    // which is roughly 128 * n * r bytes (plus the small Salsa20/8 block array).
    const auto maxmem = static_cast<quint64>(128) * n * r + static_cast<quint64>(128) * r * p + 4096;
    if (EVP_PBE_scrypt(passwordUtf8.constData(), passwordUtf8.size(),
                       reinterpret_cast<const unsigned char *>(salt.constData()), salt.size(), n, r, p, maxmem,
                       reinterpret_cast<unsigned char *>(key.data()), key.size()) != 1) {
        return QByteArray();
    }
    return key;
}

bool PasswordHasher::costParamsAreSane(int n, int r, int p)
{
    // Bounds adopted during review: n in [1024, 2**20] and a power of two, r in [1, 32],
    // p in [1, 16]. Anything else is rejected before we allocate or derive for it.
    return n >= 1024 && n <= (1 << 20) && (n & (n - 1)) == 0 && r >= 1 && r <= 32 && p >= 1 && p <= 16;
}

QString PasswordHasher::generatePasswordVerifier(const QString &password)
{
    const QByteArray salt = CryptoUtil::randomBytes(SCRYPT_SALT_LENGTH);
    const QByteArray verifier = deriveKey(password, salt, SCRYPT_N, SCRYPT_R, SCRYPT_P);
    return QString("$scrypt$%1$%2$%3$%4$%5")
        .arg(SCRYPT_N)
        .arg(SCRYPT_R)
        .arg(SCRYPT_P)
        .arg(QString(salt.toBase64()))
        .arg(QString(verifier.toBase64()));
}

PasswordVerifier PasswordHasher::parsePasswordVerifier(const QString &stored)
{
    PasswordVerifier result;
    const QStringList parts = stored.split("$");
    if (parts.size() != 7 || parts.at(1) != "scrypt") {
        return result;
    }

    bool ok = false;
    const int n = parts.at(2).toInt(&ok);
    if (!ok) {
        return result;
    }
    const int r = parts.at(3).toInt(&ok);
    if (!ok) {
        return result;
    }
    const int p = parts.at(4).toInt(&ok);
    if (!ok || !costParamsAreSane(n, r, p)) {
        return result;
    }

    const QByteArray salt = QByteArray::fromBase64(parts.at(5).toUtf8());
    const QByteArray verifier = QByteArray::fromBase64(parts.at(6).toUtf8());
    if (salt.isEmpty() || verifier.size() != SCRYPT_VERIFIER_LENGTH) {
        return result;
    }

    result.format = PasswordFormat::Scrypt;
    result.n = n;
    result.r = r;
    result.p = p;
    result.salt = salt;
    result.verifier = verifier;
    result.isValid = true;
    return result;
}

bool PasswordHasher::isLegacyFormat(const QString &stored)
{
    return !stored.startsWith("$");
}

bool PasswordHasher::verifyPassword(const QString &password, const QString &storedPasswordData)
{
    if (isLegacyFormat(storedPasswordData)) {
        return storedPasswordData == computeHash(password, storedPasswordData.left(16));
    }

    const PasswordVerifier verifier = parsePasswordVerifier(storedPasswordData);
    if (!verifier.isValid) {
        return false;
    }
    const QByteArray derived = deriveKey(password, verifier.salt, verifier.n, verifier.r, verifier.p);
    return !derived.isEmpty() && constantTimeEquals(derived, verifier.verifier);
}

QByteArray PasswordHasher::computeResponse(const QByteArray &key, const QByteArray &nonce)
{
    QByteArray response(EVP_MAX_MD_SIZE, '\0');
    unsigned int responseLength = 0;
    if (HMAC(EVP_sha256(), key.constData(), key.size(), reinterpret_cast<const unsigned char *>(nonce.constData()),
             nonce.size(), reinterpret_cast<unsigned char *>(response.data()), &responseLength) == nullptr) {
        qFatal("PasswordHasher::computeResponse: HMAC failed");
    }
    response.resize(responseLength);
    return response;
}

bool PasswordHasher::constantTimeEquals(const QByteArray &a, const QByteArray &b)
{
    if (a.size() != b.size()) {
        return false;
    }
    return CRYPTO_memcmp(a.constData(), b.constData(), a.size()) == 0;
}
