#include "passwordhasher.h"

#include <QCryptographicHash>
#include <libcockatrice/utility/cryptoutil.h>

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
