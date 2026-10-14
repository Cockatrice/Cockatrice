#include "crash_reporter.h"

#if COCKATRICE_WITH_CRASH_REPORTING
#include <sentry.h>
#endif

#include <QDir>
#include <QRegularExpression>
#include <QUrl>
#include <libcockatrice/settings/debug_settings.h>

void CrashReporter::init(const QString &dsn, const QString &release, const QString &databasePath, int consentMode)
{
    shutdown();
    if (dsn.isEmpty() || consentMode == static_cast<int>(DebugSettings::CrashReportingMode::NeverSend)) {
        return;
    }
    // Only created when the SDK will actually run: no-DSN launches stay side-effect free.
#if COCKATRICE_WITH_CRASH_REPORTING
    QDir().mkpath(databasePath);
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, dsn.toUtf8().constData());
    sentry_options_set_release(options, release.toUtf8().constData());
    sentry_options_set_database_path(options, databasePath.toUtf8().constData());
    ready = (sentry_init(options) == 0);
#else
    Q_UNUSED(release);
    Q_UNUSED(databasePath);
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
        QString token = match.captured(0);
        QString replacement = token;
        if (token.startsWith(QStringLiteral("cockatrice://"))) {
            QUrl parsed(token);
            replacement = parsed.scheme() + QStringLiteral("://") + parsed.host();
        }
        redacted.append(replacement);
        pos = match.capturedEnd();
    }
    redacted.append(line.mid(pos));
    return redacted;
}
