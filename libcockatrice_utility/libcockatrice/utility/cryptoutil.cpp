#include "cryptoutil.h"

#include <openssl/rand.h>

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
} // namespace CryptoUtil
