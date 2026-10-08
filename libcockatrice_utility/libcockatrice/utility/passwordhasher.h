#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <QString>

class PasswordHasher
{
public:
    static QString computeHash(const QString &password, const QString &salt);
    static QString generateRandomSalt(const int len = 16);
    static QString generateActivationToken();
};

#endif
