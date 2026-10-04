#ifndef CRYPTOUTIL_H
#define CRYPTOUTIL_H

#include <QByteArray>
#include <QtTypes>

namespace CryptoUtil
{
QByteArray randomBytes(int count);
quint64 randomUInt64();
} // namespace CryptoUtil

#endif
