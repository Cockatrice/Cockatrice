/**
 * @file crash_reporter.h
 * @ingroup Core
 */

#ifndef CRASH_REPORTER_H
#define CRASH_REPORTER_H

#include <QString>

#ifndef COCKATRICE_WITH_CRASH_REPORTING
#define COCKATRICE_WITH_CRASH_REPORTING 0
#endif

#ifndef COCKATRICE_SENTRY_DSN
#define COCKATRICE_SENTRY_DSN ""
#endif

/**
 * @brief Sentry-backed crash-report facade, owned by main().
 *
 * Single responsibility: init/shutdown the sentry-native SDK behind consent
 * gating, plus the shared log redaction helpers. Increment 1 wires init as a
 * no-op: an empty DSN (or Never consent) skips the SDK entirely, so the mock
 * dialog fallback behaves exactly as before. The real send path is a later
 * increment. All SDK calls are guarded so OFF builds compile to no-ops.
 */
class CrashReporter
{
public:
    CrashReporter() = default;
    CrashReporter(const CrashReporter &) = delete;
    CrashReporter &operator=(const CrashReporter &) = delete;
    ~CrashReporter() = default;

    void init(const QString &dsn, const QString &release, const QString &databasePath, int consentMode);
    [[nodiscard]] bool isReady() const;
    void shutdown();

    static QString sanitizeLogLine(const QString &line);
    static QString redactCockatriceUrls(const QString &line);

private:
    bool ready = false;
};

#endif
