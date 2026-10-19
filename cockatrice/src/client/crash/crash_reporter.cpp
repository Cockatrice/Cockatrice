#include "crash_reporter.h"

#if COCKATRICE_WITH_CRASH_REPORTING
#include <sentry.h>
#endif

#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QUrl>
#include <libcockatrice/settings/personal_settings.h>

Q_LOGGING_CATEGORY(CrashReportLog, "crash_report");

bool CrashReporter::isReportingEnabled(const QString &dsn, PersonalSettings::CrashReportingMode consentMode)
{
    return !dsn.isEmpty() && consentMode != PersonalSettings::CrashReportingMode::NeverSend;
}

void CrashReporter::init(const QString &dsn,
                         const QString &release,
                         const QString &databasePath,
                         PersonalSettings::CrashReportingMode consentMode)
{
    shutdown();
    sentryDatabasePath = databasePath;
    if (!isReportingEnabled(dsn, consentMode)) {
        return;
    }
    // Only created when the SDK will actually run: no-DSN launches stay side-effect free.
#if COCKATRICE_WITH_CRASH_REPORTING
    QDir().mkpath(sentryDatabasePath);
    if (consentMode != PersonalSettings::CrashReportingMode::AlwaysSend) {
        // sentry-native persists consent to <database>/user-consent and reloads it in sentry_init
        // before old runs are processed, so a stale "given" from an earlier AlwaysSend run (or an
        // earlier Send in Ask mode) would upload the next crash before the dialog asks. Our
        // CrashReportingMode is the source of truth, so clear the persisted state first; asking
        // afterwards re-establishes it via giveConsent()/revokeConsent().
        QFile::remove(sentryDatabasePath + QStringLiteral("/user-consent"));
    }
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, dsn.toUtf8().constData());
    sentry_options_set_release(options, release.toUtf8().constData());
    sentry_options_set_database_path(options, sentryDatabasePath.toUtf8().constData());
    // Consent-gated uploads: crashes are captured into the local database but
    // nothing leaves the machine until giveConsent().
    sentry_options_set_require_user_consent(options, 1);
    ready = (sentry_init(options) == 0);
#else
    Q_UNUSED(release);
    ready = false;
#endif
}

bool CrashReporter::isReady() const
{
    return ready;
}

void CrashReporter::shutdown()
{
#if COCKATRICE_WITH_CRASH_REPORTING
    if (ready) {
        sentry_close();
    }
#endif
    ready = false;
}

void CrashReporter::giveConsent()
{
#if COCKATRICE_WITH_CRASH_REPORTING
    if (ready) {
        sentry_user_consent_give();
    }
#endif
}

void CrashReporter::revokeConsent()
{
#if COCKATRICE_WITH_CRASH_REPORTING
    if (ready) {
        sentry_user_consent_revoke();
    }
#endif
}

QString CrashReporter::sanitizeLogLine(const QString &line)
{
    static const auto colorEscapeCodePattern = QRegularExpression(QStringLiteral("\033\\[[0-9;]*m"));

    QString sanitized = line;
    sanitized.replace(colorEscapeCodePattern, "");
    return sanitized;
}

QString CrashReporter::redactCockatriceUrls(const QString &line)
{
    static auto urlPattern = QRegularExpression(QStringLiteral("cockatrice://\\S+"));

    // Single pass over the original line: rescanning the replaced text would match a redacted
    // secret-free URL again and loop forever.
    QString redacted;
    redacted.reserve(line.size());
    qsizetype pos = 0;
    QRegularExpressionMatchIterator it = urlPattern.globalMatch(line);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        redacted.append(line.mid(pos, match.capturedStart() - pos));
        // The pattern only matches cockatrice:// URLs, so the check is folded into the parse:
        // scheme://host, secrets dropped.
        const QUrl parsed(match.captured(0));
        redacted.append(parsed.scheme() + QStringLiteral("://") + parsed.host());
        pos = match.capturedEnd();
    }
    redacted.append(line.mid(pos));
    return redacted;
}
