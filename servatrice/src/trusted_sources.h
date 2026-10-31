#ifndef TRUSTED_SOURCES_H
#define TRUSTED_SOURCES_H

#include <QHostAddress>
#include <QString>

/**
 * @brief True if @p address is listed in the comma-separated @p trustedSources (security/trusted_sources).
 *
 * Entries are parsed as addresses and compared exactly, never as substrings, so
 * 27.0.0.1 does not match 127.0.0.1. IPv4-mapped IPv6 addresses compare equal to
 * their IPv4 form: a dual-stack listener reports a local proxy as ::ffff:127.0.0.1,
 * which matches a 127.0.0.1 entry. Malformed entries and null addresses never match.
 */
bool isTrustedSource(const QString &trustedSources, const QHostAddress &address);

/** @brief Overload for an address in string form; an unparsable string never matches. */
bool isTrustedSource(const QString &trustedSources, const QString &address);

#endif
