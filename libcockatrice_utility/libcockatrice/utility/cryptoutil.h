#ifndef CRYPTOUTIL_H
#define CRYPTOUTIL_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

namespace CryptoUtil
{
QByteArray randomBytes(int count);
quint64 randomUInt64();
/** @brief Seal a secret with AES-256-GCM as "enc1:<b64 nonce>:<b64 ciphertext+tag>".
 *  Returns an empty string unless @p key is a valid 32-byte key and the cipher
 *  completes. @p aad is authenticated but not stored (bind the ciphertext to its
 *  context, e.g. the user name it belongs to). */
QString encryptSecret(const QByteArray &plaintext, const QByteArray &key, const QByteArray &aad = {});
/** @brief Open a string produced by encryptSecret(). Returns an empty array on any
 *  failure: unknown format, malformed base64, wrong key, wrong aad, or tampering. */
QByteArray decryptSecret(const QString &sealed, const QByteArray &key, const QByteArray &aad = {});
} // namespace CryptoUtil

#endif
