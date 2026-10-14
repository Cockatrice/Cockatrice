/**
 * @file crash_reporter.h
 * @ingroup Core
 */

#ifndef CRASH_REPORTER_H
#define CRASH_REPORTER_H

#include <QLoggingCategory>
#include <QString>

#ifndef COCKATRICE_WITH_CRASH_REPORTING
#define COCKATRICE_WITH_CRASH_REPORTING 0
#endif

#ifndef COCKATRICE_SENTRY_DSN
#define COCKATRICE_SENTRY_DSN ""
#endif

Q_DECLARE_LOGGING_CATEGORY(CrashReportLog);

/**
 * @brief Sentry-backed crash-report facade, owned by main().
 *
 * Single responsibility: init/shutdown the sentry-native SDK behind consent
 * gating, plus the shared log redaction helpers. An empty DSN (or Never
 * consent) skips the SDK entirely, so the mock dialog fallback behaves exactly
 * as before. With a DSN, init() arms require-user-consent and before_send
 * scrubbing; giveConsent()/revokeConsent() gate uploads, and sendReport()
 * captures a redacted message event with the log preview attached. All SDK
 * calls are guarded so OFF builds compile to no-ops.
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
    void giveConsent();
    void revokeConsent();
    bool sendReport(const QString &redactedPreview);

    static QString sanitizeLogLine(const QString &line);
    static QString redactCockatriceUrls(const QString &line);

private:
    bool ready = false;
    QString sentryDatabasePath;
};

#endif
