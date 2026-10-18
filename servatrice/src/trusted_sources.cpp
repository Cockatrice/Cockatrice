#include "trusted_sources.h"

#include <QStringList>

bool isTrustedSource(const QString &trustedSources, const QHostAddress &address)
{
    if (address.isNull()) {
        return false;
    }
    const QStringList entries = trustedSources.split(',', Qt::SkipEmptyParts);
    for (const QString &entry : entries) {
        const QHostAddress trusted(entry.trimmed());
        if (!trusted.isNull() && trusted.isEqual(address, QHostAddress::ConvertV4MappedToIPv4)) {
            return true;
        }
    }
    return false;
}

bool isTrustedSource(const QString &trustedSources, const QString &address)
{
    return isTrustedSource(trustedSources, QHostAddress(address.trimmed()));
}
