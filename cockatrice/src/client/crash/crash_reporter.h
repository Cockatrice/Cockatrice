/**
 * @file crash_reporter.h
 * @ingroup Core
 */

#ifndef CRASH_REPORTER_H
#define CRASH_REPORTER_H

#include <QLoggingCategory>
#include <QString>
#include <memory>

#ifndef COCKATRICE_WITH_CRASH_REPORTING
#define COCKATRICE_WITH_CRASH_REPORTING 0
#endif

#ifndef COCKATRICE_SENTRY_DSN
#define COCKATRICE_SENTRY_DSN ""
#endif

class QLockFile;

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
 * calls are guarded so OFF builds compile to no-ops. Session tracking
 * (beginSession/endSession) is SDK-independent: a QLockFile per process in the
 * database dir detects unclean exits cross-platform, replacing the legacy
 * Windows-only .dmp scan. The held lock makes the reporter non-copyable and
 * non-movable.
 */
class CrashReporter
{
public:
    CrashReporter();
    CrashReporter(const CrashReporter &) = delete;
    CrashReporter &operator=(const CrashReporter &) = delete;
    CrashReporter(CrashReporter &&) = delete;
    CrashReporter &operator=(CrashReporter &&) = delete;
    ~CrashReporter();

    void init(const QString &dsn, const QString &release, const QString &databasePath, int consentMode);
    [[nodiscard]] bool isReady() const;
    void shutdown();
    void giveConsent();
    void revokeConsent();
    bool sendReport(const QString &redactedPreview);

    /**
     * @brief Claim this process's session lock, reporting a previous unclean exit.
     * @return true if a leftover session lock from a dead process (or an unreadable
     * garbage file) was found and cleaned; live concurrent instances are ignored.
     */
    bool beginSession(const QString &dir);
    void endSession();

    static QString sanitizeLogLine(const QString &line);
    static QString redactCockatriceUrls(const QString &line);

private:
    bool ready = false;
    QString sentryDatabasePath;
    QString sessionLockPath;
    std::unique_ptr<QLockFile> sessionLock;
};

#endif
