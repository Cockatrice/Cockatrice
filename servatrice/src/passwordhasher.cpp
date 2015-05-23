#include "passwordhasher.h"

#if QT_VERSION < 0x050000
    #include <stdio.h>
    #include <string.h>
    #include <gcrypt.h>
#else
    #include <QCryptographicHash>
#endif

#include "rng_sfmt.h"

void PasswordHasher::initialize()
{
#if QT_VERSION < 0x050000
    // These calls are required by libgcrypt before we use any of its functions.
    gcry_check_version(0);
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
}

#if QT_VERSION < 0x050000
QString PasswordHasher::computeHash(const QString &password, const QString &salt)
{
    const int algo = GCRY_MD_SHA512;
    const int rounds = 1000;

    QByteArray passwordBuffer = (salt + password).toUtf8();
    int hashLen = gcry_md_get_algo_dlen(algo);
    char *hash = new char[hashLen], *tmp = new char[hashLen];
    gcry_md_hash_buffer(algo, hash, passwordBuffer.data(), passwordBuffer.size());
    for (int i = 1; i < rounds; ++i) {
        memcpy(tmp, hash, hashLen);
        gcry_md_hash_buffer(algo, hash, tmp, hashLen);
    }
    QString hashedPass = salt + QString(QByteArray(hash, hashLen).toBase64());
    delete[] tmp;
    delete[] hash;
    return hashedPass;
}
#else
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
#endif

QString PasswordHasher::generateRandomSalt(const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    QString ret;
    int size = sizeof(alphanum) - 1;

    for (int i = 0; i < len; ++i) {
        ret.append(alphanum[rng->rand(0, size)]);
    }

    return ret;
}

QString PasswordHasher::generateActivationToken()
{
    return QCryptographicHash::hash(generateRandomSalt().toUtf8(), QCryptographicHash::Md5).toBase64().left(16);
}