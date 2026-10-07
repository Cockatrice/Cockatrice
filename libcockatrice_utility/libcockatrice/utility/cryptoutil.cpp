#include "cryptoutil.h"

#include <QStringList>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Sealed secret layout: "enc1:<base64 nonce>:<base64 ciphertext+tag>".
static constexpr int SECRET_KEY_LENGTH = 32;
static constexpr int SECRET_NONCE_LENGTH = 12;
static constexpr int SECRET_TAG_LENGTH = 16;
static const char *SECRET_FORMAT_PREFIX = "enc1";

namespace CryptoUtil
{
QByteArray randomBytes(int count)
{
    QByteArray bytes(count, '\0');
    if (RAND_bytes(reinterpret_cast<unsigned char *>(bytes.data()), count) != 1) {
        // Randomness failure is fatal: never fall back to a predictable source.
        qFatal("CryptoUtil::randomBytes: RAND_bytes failed");
    }
    return bytes;
}

quint64 randomUInt64()
{
    quint64 value;
    if (RAND_bytes(reinterpret_cast<unsigned char *>(&value), sizeof(value)) != 1) {
        qFatal("CryptoUtil::randomUInt64: RAND_bytes failed");
    }
    return value;
}

QString encryptSecret(const QByteArray &plaintext, const QByteArray &key, const QByteArray &aad)
{
    if (key.size() != SECRET_KEY_LENGTH) {
        return {};
    }
    const QByteArray nonce = randomBytes(SECRET_NONCE_LENGTH);
    QByteArray ciphertext(plaintext.size() + SECRET_TAG_LENGTH, Qt::Uninitialized);
    QByteArray tag(SECRET_TAG_LENGTH, Qt::Uninitialized);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        return {};
    }
    int written = 0;
    int finalWritten = 0;
    bool ok = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
              EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) == 1 &&
              EVP_EncryptInit_ex(ctx, nullptr, nullptr, reinterpret_cast<const unsigned char *>(key.constData()),
                                 reinterpret_cast<const unsigned char *>(nonce.constData())) == 1;
    if (ok && !aad.isEmpty()) {
        ok = EVP_EncryptUpdate(ctx, nullptr, &written, reinterpret_cast<const unsigned char *>(aad.constData()),
                               aad.size()) == 1;
    }
    if (ok) {
        ok = EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(ciphertext.data()), &written,
                               reinterpret_cast<const unsigned char *>(plaintext.constData()), plaintext.size()) == 1;
    }
    if (ok) {
        ok = EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(ciphertext.data() + written), &finalWritten) ==
                 1 &&
             EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, SECRET_TAG_LENGTH, tag.data()) == 1;
    }
    EVP_CIPHER_CTX_free(ctx);
    if (!ok) {
        return {};
    }
    ciphertext.truncate(written + finalWritten);
    ciphertext.append(tag);

    return QString::fromLatin1(SECRET_FORMAT_PREFIX) + QLatin1Char(':') + nonce.toBase64() + QLatin1Char(':') +
           ciphertext.toBase64();
}

QByteArray decryptSecret(const QString &sealed, const QByteArray &key, const QByteArray &aad)
{
    if (key.size() != SECRET_KEY_LENGTH) {
        return {};
    }
    const QStringList parts = sealed.split(QLatin1Char(':'));
    if (parts.size() != 3 || parts.at(0) != QLatin1String(SECRET_FORMAT_PREFIX)) {
        return {};
    }
    const QByteArray nonce = QByteArray::fromBase64(parts.at(1).toUtf8());
    const QByteArray ciphertext = QByteArray::fromBase64(parts.at(2).toUtf8());
    if (nonce.size() != SECRET_NONCE_LENGTH || ciphertext.size() < SECRET_TAG_LENGTH) {
        return {};
    }
    const int plaintextLength = ciphertext.size() - SECRET_TAG_LENGTH;
    QByteArray plaintext(qMax(plaintextLength, 1), Qt::Uninitialized);
    QByteArray tag = ciphertext.right(SECRET_TAG_LENGTH);
    const QByteArray body = ciphertext.left(plaintextLength);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        return {};
    }
    int written = 0;
    int finalWritten = 0;
    bool ok = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
              EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) == 1 &&
              EVP_DecryptInit_ex(ctx, nullptr, nullptr, reinterpret_cast<const unsigned char *>(key.constData()),
                                 reinterpret_cast<const unsigned char *>(nonce.constData())) == 1;
    if (ok && !aad.isEmpty()) {
        ok = EVP_DecryptUpdate(ctx, nullptr, &written, reinterpret_cast<const unsigned char *>(aad.constData()),
                               aad.size()) == 1;
    }
    if (ok) {
        // The tag must be set before the final call, which authenticates it; a
        // mismatch makes EVP_DecryptFinal_ex fail and the secret stays opaque.
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data()) == 1 &&
             EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(plaintext.data()), &written,
                               reinterpret_cast<const unsigned char *>(body.constData()), body.size()) == 1;
    }
    if (ok) {
        ok =
            EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(plaintext.data() + written), &finalWritten) == 1;
    }
    EVP_CIPHER_CTX_free(ctx);
    if (!ok) {
        return {};
    }
    plaintext.truncate(written + finalWritten);
    return plaintext;
}
} // namespace CryptoUtil
