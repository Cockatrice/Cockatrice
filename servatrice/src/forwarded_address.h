#ifndef FORWARDED_ADDRESS_H
#define FORWARDED_ADDRESS_H

#include <QByteArray>
#include <QHostAddress>

/**
 * @brief Parses the real client address out of a forwarded header value (X-Real-IP, X-Forwarded-For).
 *
 * A comma-separated list resolves to its rightmost usable entry. That entry was appended by the
 * proxy closest to the server and is the only one a direct client cannot choose, since the earlier
 * entries are passed through verbatim from whatever the client sent.
 *
 * @return the parsed address, or a null QHostAddress when the value holds none.
 */
QHostAddress parseForwardedAddress(const QByteArray &headerValue);

#endif
