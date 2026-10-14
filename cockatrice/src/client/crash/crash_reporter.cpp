#include "crash_reporter.h"

#if COCKATRICE_WITH_CRASH_REPORTING
#include <sentry.h>
#endif

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QUrl>
#include <cstring>
#include <libcockatrice/settings/personal_settings.h>
#include <string>

Q_LOGGING_CATEGORY(CrashReportLog, "crash_report");

#if COCKATRICE_WITH_CRASH_REPORTING
namespace
{

constexpr char CockatriceUrlScheme[] = "cockatrice://";

/**
 * @brief Qt-free cockatrice:// secret scrubber for the before_send hook.
 *
 * before_send does not run in async-signal context on our backends (breakpad invokes it on the
 * crashing thread outside the signal handler, and crashpad/native crashes bypass the hook
 * entirely), but it still avoids QString/QRegularExpression so it stays safe to call from the
 * crash-handler thread. Truncates each URL at the first terminator, keeping scheme://host like
 * redactCockatriceUrls does. (std::string allocates, so this is not strictly signal-safe.)
 */
std::string scrubCockatriceUrls(const char *input)
{
    std::string out;
    const char *pos = input;
    const char *scheme = CockatriceUrlScheme;
    const size_t schemeLen = std::strlen(scheme);
    while (*pos != '\0') {
        const char *found = std::strstr(pos, scheme);
        if (found == nullptr) {
            out.append(pos);
            break;
        }
        out.append(pos, found);
        const char *hostEnd = found + schemeLen;
        while (*hostEnd != '\0' && *hostEnd != '/' && *hostEnd != '?' && *hostEnd != '#' && *hostEnd != ' ' &&
               *hostEnd != '\t' && *hostEnd != '\n' && *hostEnd != '\'' && *hostEnd != '"' && *hostEnd != '<' &&
               *hostEnd != '>') {
            ++hostEnd;
        }
        out.append(found, hostEnd);
        pos = hostEnd;
    }
    return out;
}

void scrubStringValue(sentry_value_t holder, const char *key)
{
    sentry_value_t current = sentry_value_get_by_key(holder, key);
    if (sentry_value_get_type(current) != SENTRY_VALUE_TYPE_STRING) {
        return;
    }
    const char *raw = sentry_value_as_string(current);
    if (raw == nullptr || std::strstr(raw, CockatriceUrlScheme) == nullptr) {
        return;
    }
    sentry_value_set_by_key(holder, key, sentry_value_new_string(scrubCockatriceUrls(raw).c_str()));
}

/**
 * @brief Scrub trivially reachable string values inside a breadcrumb data object.
 */
int scrubDataEntry(const char *key, sentry_value_t value, void *userdata)
{
    if (sentry_value_get_type(value) != SENTRY_VALUE_TYPE_STRING) {
        return 0;
    }
    const char *raw = sentry_value_as_string(value);
    if (raw == nullptr || std::strstr(raw, CockatriceUrlScheme) == nullptr) {
        return 0;
    }
    auto *holder = static_cast<sentry_value_t *>(userdata);
    sentry_value_set_by_key(*holder, key, sentry_value_new_string(scrubCockatriceUrls(raw).c_str()));
    return 0;
}

void scrubDataObject(sentry_value_t data)
{
    if (sentry_value_get_type(data) != SENTRY_VALUE_TYPE_OBJECT) {
        return;
    }
    sentry_value_foreach_key_value(data, scrubDataEntry, &data);
}

void scrubCrumbList(sentry_value_t crumbs)
{
    const size_t count = sentry_value_get_length(crumbs);
    for (size_t i = 0; i < count; ++i) {
        sentry_value_t crumb = sentry_value_get_by_index(crumbs, i);
        if (sentry_value_get_type(crumb) != SENTRY_VALUE_TYPE_OBJECT) {
            continue;
        }
        scrubStringValue(crumb, "message");
        scrubDataObject(sentry_value_get_by_key(crumb, "data"));
    }
}

void scrubBreadcrumbs(sentry_value_t event)
{
    // The SDK merges scope breadcrumbs onto the event as a plain list (sentry__value_merge_breadcrumbs
    // in sentry_scope.c); the { "values": [...] } envelope shape only exists on the wire. Accept both
    // so the scrubbing runs on the actual merged form.
    sentry_value_t breadcrumbs = sentry_value_get_by_key(event, "breadcrumbs");
    if (sentry_value_get_type(breadcrumbs) == SENTRY_VALUE_TYPE_LIST) {
        scrubCrumbList(breadcrumbs);
        return;
    }
    if (sentry_value_get_type(breadcrumbs) == SENTRY_VALUE_TYPE_OBJECT) {
        sentry_value_t values = sentry_value_get_by_key(breadcrumbs, "values");
        if (sentry_value_get_type(values) == SENTRY_VALUE_TYPE_LIST) {
            scrubCrumbList(values);
        }
    }
}

/**
 * @brief before_send hook: scrub cockatrice:// secrets from the event.
 *
 * Defense in depth — payloads we build are pre-redacted. No breadcrumbs are recorded today (the Qt
 * integration stays off), so this currently covers the top-level message and logentry.formatted,
 * plus any breadcrumb messages/data that reach the merged event in the future. Native crash
 * minidumps bypass this hook regardless of backend: breakpad captures them in-process without
 * invoking before_send, and fast-fail crashes on Windows skip the local handler, so before_send is
 * never invoked for those.
 */
sentry_value_t scrubEventBeforeSend(sentry_value_t event, sentry_hint_t *hint, void *userData)
{
    Q_UNUSED(hint);
    Q_UNUSED(userData);
    scrubStringValue(event, "message");
    sentry_value_t logentry = sentry_value_get_by_key(event, "logentry");
    if (sentry_value_get_type(logentry) == SENTRY_VALUE_TYPE_OBJECT) {
        scrubStringValue(logentry, "formatted");
    }
    scrubBreadcrumbs(event);
    return event;
}

} // namespace
#endif

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
    sentry_options_set_before_send(options, scrubEventBeforeSend, nullptr);
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
    // Revoke (not reset): an explicit refusal blocks uploads; reset would
    // return to unknown, which blocks uploads too but loses the decision.
    if (ready) {
        sentry_user_consent_revoke();
    }
#endif
}

bool CrashReporter::sendReport(const QString &redactedPreview)
{
#if COCKATRICE_WITH_CRASH_REPORTING
    if (!ready) {
        return false;
    }
    // Local scope (not the global one): the log preview rides only on this event. A global
    // sentry_attach_bytes would linger and ride along on every later event in the session,
    // including a real native crash later on — and with the crashpad backend byte attachments
    // are written to disk for exactly that. In-memory bytes also mean no report files land in
    // the Sentry DB dir, so there is no disk growth and no same-second filename collision.
    sentry_scope_t *scope = sentry_scope_new();
    if (scope == nullptr) {
        qCWarning(CrashReportLog) << "Could not create scope for crash report";
        return false;
    }
    const QByteArray previewBytes = redactedPreview.toUtf8();
    const sentry_uuid_t attachmentId = sentry_scope_attach_bytes(
        scope, previewBytes.constData(), static_cast<size_t>(previewBytes.size()), "crash-log.txt");
    const bool attachOk = sentry_uuid_is_nil(&attachmentId) == 0;
    if (!attachOk) {
        qCWarning(CrashReportLog) << "Could not attach crash log to crash report";
    }

    // Release is attached automatically from the init options. Deliberately no
    // sentry_set_user call: the report must carry no PII. The dialog collects
    // no contact info, so the message event + attachment is the whole payload
    // (sentry_value_new_feedback/sentry_capture_feedback exist but stay unused).
    sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_ERROR, "crash-report", "Crash report");
    const sentry_uuid_t eventId = sentry_scope_capture_event(scope, event, nullptr);
    sentry_scope_free(scope);
    if (sentry_uuid_is_nil(&eventId)) {
        qCWarning(CrashReportLog) << "Failed to capture crash report event";
        return false;
    }
    char eventIdStr[37] = {};
    sentry_uuid_as_string(&eventId, eventIdStr);
    // Capture only queues the envelope to the background transport: a network failure stays
    // invisible here, so true means "queued", not "delivered". The Save fallback is offered only
    // when the local capture itself fails.
    qCInfo(CrashReportLog) << "Crash report queued, event id:" << eventIdStr;
    // A failed attachment still captures the event above, but reports failure
    // so callers offer the local-save fallback instead of silent success.
    return attachOk;
#else
    Q_UNUSED(redactedPreview);
    return false;
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
