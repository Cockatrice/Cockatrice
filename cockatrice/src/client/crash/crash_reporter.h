/**
 * @file crash_reporter.h
 * @ingroup Core
 */

#ifndef CRASH_REPORTER_H
#define CRASH_REPORTER_H

#include <QByteArray>
#include <QLoggingCategory>
#include <QString>
#include <QStringList>
#include <libcockatrice/settings/personal_settings.h>
#include <memory>

#ifndef COCKATRICE_WITH_CRASH_REPORTING
#define COCKATRICE_WITH_CRASH_REPORTING 0
#endif

#ifndef COCKATRICE_SENTRY_DSN
#define COCKATRICE_SENTRY_DSN ""
#endif

#ifndef COCKATRICE_SENTRY_RELEASE
#define COCKATRICE_SENTRY_RELEASE ""
#endif

class QLockFile;

Q_DECLARE_LOGGING_CATEGORY(CrashReportLog);

/**
 * @brief Sentry-backed crash-report facade, owned by main().
 *
 * An empty DSN (or Never consent) skips the SDK entirely, so the mock dialog
 * fallback behaves exactly as before. All SDK calls are guarded so OFF builds
 * compile to no-ops.
 *
 * @par Consent gating
 * init() arms require-user-consent and before_send scrubbing. Stale persisted
 * consent from a previous AlwaysSend run is cleared before init when the mode is
 * not AlwaysSend, and cached crash envelopes are held for the dialog instead of
 * being dropped. giveConsent()/revokeConsent() gate uploads.
 *
 * @par Envelope and log helpers
 * attachToPendingCrashEnvelopes() folds the redacted log and the optional user
 * note into the pending native crash event(s) before consent releases them, so
 * dump, stack, log and note arrive as ONE Sentry event. sendReport() is the
 * fallback for unclean exits that produced no native crash event, capturing a
 * redacted message event with the log preview attached. The log redaction
 * helpers are static and SDK-independent.
 *
 * @par Session tracking
 * beginSession()/endSession() are SDK-independent: a QLockFile per process in
 * the database dir detects unclean exits cross-platform, replacing the legacy
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

    void init(const QString &dsn,
              const QString &release,
              const QString &databasePath,
              PersonalSettings::CrashReportingMode consentMode);
    [[nodiscard]] bool isReady() const;
    void shutdown();
    void giveConsent();
    void revokeConsent();
    bool sendReport(const QString &redactedPreview, const QString &userNote = QString());

    /**
     * @brief Append the redacted log (and optional user note) to every pending
     * native crash envelope in the local database, so the crashed event uploads
     * with dump, stack, log and note together instead of the log arriving as a
     * separate event that has to be correlated by hand. Both storage locations
     * are covered: consent-held envelopes in <database>/cache (Ask mode) and
     * in-flight envelopes dumped into <database>/<run>.run (AlwaysSend mode).
     *
     * For the AlwaysSend variant run this BEFORE init() (static overload): the
     * startup replay of *.run envelopes happens inside sentry_init. SDK-independent
     * plain-file surgery on the Sentry envelope format, so it stays unit-testable
     * in OFF builds.
     *
     * @return the number of envelopes that received the attachments.
     */
    int attachToPendingCrashEnvelopes(const QString &redactedLogText, const QString &userNoteText);
    static int attachToPendingCrashEnvelopes(const QString &databasePath,
                                             const QString &redactedLogText,
                                             const QString &userNoteText);

    /**
     * @brief Append one attachment item to a cached Sentry envelope file.
     *
     * An envelope is a newline-separated stream of item-header-line + payload
     * pairs. The retry transport replays cached envelopes byte-for-byte
     * (sentry-native reads them raw and serializes raw envelopes back
     * verbatim), so appending well-formed items associates them with the event
     * already in the file; the first line (event id header) is never touched.
     *
     * @param filename ASCII-only item filename, embedded verbatim in the header.
     */
    static bool
    appendAttachmentToEnvelope(const QString &envelopePath, const QString &filename, const QByteArray &payload);

    /**
     * @brief Pending crash envelopes in <database>: the consent-held files in
     * <database>/cache and the in-flight files in <database>/<run>.run.
     *
     * Matches the cache filename conventions sentry-native accepts (see
     * sentry__parse_cache_filename): bare "<uuid>.envelope" and retry
     * "<ts>-<count>-<uuid>.envelope"; run dirs accept plain "<uuid>.envelope"
     * minus the SDK's own "__sentry-crash.envelope" marker.
     */
    static QStringList pendingEnvelopePaths(const QString &databasePath);

    /**
     * @brief SDK-independent gating decision, unit-testable in OFF builds.
     */
    static bool isReportingEnabled(const QString &dsn, PersonalSettings::CrashReportingMode consentMode);

    /**
     * @brief Whether a DSN has the shape the build and the SDK require:
     * <scheme>://<key>@<host>/<project-id>.
     *
     * Deliberately the same unanchored pattern cmake/CrashReporting.cmake matches (CMake's MATCHES
     * searches rather than anchoring), so a DSN this build accepted is never rejected here. The SDK
     * refuses to even build a request for a DSN it considers invalid, so an unchecked typo disables
     * uploading with no error anywhere.
     */
    static bool isDsnWellFormed(const QString &dsn);

    /**
     * @brief Human-readable destination of a DSN, e.g. "o0.ingest.sentry.io (project 1234567)".
     *
     * Safe to log: the public key is never part of the result. Empty for a malformed DSN.
     */
    static QString describeDsn(const QString &dsn);

    /**
     * @brief Describe the captured-for DSN of a pending envelope when it points at a different
     * destination than the DSN this session would upload it with; empty when they agree.
     *
     * sentry-native posts every envelope with the DSN held in the transport state of the session
     * doing the sending (envelopes restored from disk are raw and their own "dsn" header is never
     * read), so a cached crash envelope captured for one project can silently be uploaded to
     * another - the exact way a tester's report lands in the wrong place. Compares destinations
     * (host and project id), not raw strings, so a different key for the same project stays quiet.
     */
    static QString describeOriginMismatch(const QString &envelopePath, const QString &currentDsn);

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
