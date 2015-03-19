#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <QObject>

class PasswordHasher {
public:
	static void initialize();
	static QString computeHash(const QString &password, const QString &salt);
};

#endif
