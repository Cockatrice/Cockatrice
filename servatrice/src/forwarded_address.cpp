#include "forwarded_address.h"

#include <QString>

QHostAddress parseForwardedAddress(const QByteArray &headerValue)
{
    const QList<QByteArray> entries = headerValue.split(',');
    for (auto it = entries.crbegin(); it != entries.crend(); ++it) {
        const QByteArray entry = it->trimmed();
        if (entry.isEmpty()) {
            continue;
        }

        const QHostAddress parsed(QString::fromLatin1(entry));
        if (!parsed.isNull()) {
            return parsed;
        }
    }

    return {};
}
