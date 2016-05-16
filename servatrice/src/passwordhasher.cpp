#include "passwordhasher.h"

#include <QCryptographicHash>
#include "rng_sfmt.h"

void PasswordHasher::initialize()
{
    // dummy
}

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