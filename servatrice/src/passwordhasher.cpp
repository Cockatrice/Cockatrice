#include "passwordhasher.h"
#include <stdio.h>
#include <string.h>
#include <gcrypt.h>

#define HASH_ALGO GCRY_MD_SHA512

void PasswordHasher::initialize()
{
	// These calls are required by libgcrypt before we use any of its functions.
	gcry_check_version(0);
	gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

QString PasswordHasher::computeHash(const QString &password, const QString &salt)
{
	// concatenate salt and password
	const QByteArray passwordBuffer = (salt + password).toAscii();

	// create an array for the digest in the proper size for the chosen algorithm
	char digest[gcry_md_get_algo_dlen(HASH_ALGO)];

	// calculate the message digest of passwordBuffer
	gcry_md_hash_buffer(HASH_ALGO, digest, passwordBuffer.data(), passwordBuffer.size());

	// concatenate and return salt and digest
	return salt + QString(QByteArray(digest).toBase64());
}

