#include "passwordhasher.h"
#include <stdio.h>
#include <string.h>
#include <gcrypt.h>

void PasswordHasher::initialize()
{
	// These calls are required by libgcrypt before we use any of its functions.
	gcry_check_version(0);
	gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

QString PasswordHasher::computeHash(const QString &password, const QString &salt)
{
	const int algo = GCRY_MD_SHA512;
	const int rounds = 1000;

	QByteArray passwordBuffer = (salt + password).toAscii();
	int hashLen = gcry_md_get_algo_dlen(algo);
	char hash[hashLen], tmp[hashLen];
	gcry_md_hash_buffer(algo, hash, passwordBuffer.data(), passwordBuffer.size());
	for (int i = 1; i < rounds; ++i) {
		memcpy(tmp, hash, hashLen);
		gcry_md_hash_buffer(algo, hash, tmp, hashLen);
	}
	return salt + QString(QByteArray(hash, hashLen).toBase64());
}

