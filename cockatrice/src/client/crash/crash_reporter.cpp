#include "crash_reporter.h"

#if COCKATRICE_WITH_CRASH_REPORTING
#include <sentry.h>
#endif

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QThread>
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

namespace
{

/**
 * @brief DSN matcher shared by isDsnWellFormed() and describeDsn().
 *
 * Kept unanchored on purpose, mirroring the MATCHES in cmake/CrashReporting.cmake (CMake searches
 * rather than anchoring), so the client never rejects a DSN the build itself baked in. Compiled once
 * because QRegularExpression construction is not free and both helpers are called per pending
 * envelope.
 */
const QRegularExpression &dsnPattern()
{
    static const QRegularExpression pattern(QStringLiteral("(https?)://([^@]+)@([^/]+)/([0-9]+)"));
    return pattern;
}

} // namespace

bool CrashReporter::isDsnWellFormed(const QString &dsn)
{
    return dsnPattern().match(dsn).hasMatch();
}

QString CrashReporter::describeDsn(const QString &dsn)
{
    const QRegularExpressionMatch match = dsnPattern().match(dsn);
    if (!match.hasMatch()) {
        return QString();
    }
    // Host and project id only: the key in group 2 must never reach a log line.
    return QStringLiteral("%1 (project %2)").arg(match.captured(3), match.captured(4));
}

QString CrashReporter::describeOriginMismatch(const QString &envelopePath, const QString &currentDsn)
{
    const QString destination = describeDsn(currentDsn);
    if (destination.isEmpty()) {
        return QString();
    }

    QFile file(envelopePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    // The envelope header is the first line and is small; the cap only keeps a corrupt file from
    // being read in full.
    const QByteArray headerLine = file.readLine(4096).trimmed();
    if (headerLine.isEmpty() || !headerLine.startsWith('{')) {
        return QString();
    }
    const QString originDsn = QJsonDocument::fromJson(headerLine).object().value(QStringLiteral("dsn")).toString();
    const QString origin = describeDsn(originDsn);
    // Equal destinations are not a mismatch: a different key for the same host and project still
    // uploads where the crash was captured.
    if (origin.isEmpty() || origin == destination) {
        return QString();
    }
    return QStringLiteral("was captured for %1 but this session uploads with %2").arg(origin, destination);
}

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
        if (!dsn.isEmpty()) {
            // A deliberate choice rather than a misconfiguration, so it is not a warning. The
            // no-DSN case is reported by the caller, which is the only place that knows whether
            // the environment or the build supplied the DSN.
            qCInfo(CrashReportLog) << "crash reporting disabled by preference (Never send)";
        }
        return;
    }
    // Only created when the SDK will actually run: no-DSN launches stay side-effect free.
#if COCKATRICE_WITH_CRASH_REPORTING
    // A crash captured by an earlier run is uploaded by THIS session's transport, which posts every
    // envelope with the DSN it was armed with (sentry-native keeps the DSN in the transport state,
    // and envelopes restored from disk are raw, so their own "dsn" header is never read). When the
    // two disagree the report lands in a project nobody chose, which no other log line reveals.
    const QStringList pendingEnvelopes = pendingEnvelopePaths(sentryDatabasePath);
    for (const QString &envelopePath : pendingEnvelopes) {
        const QString mismatch = describeOriginMismatch(envelopePath, dsn);
        if (!mismatch.isEmpty()) {
            qCWarning(CrashReportLog) << "pending crash envelope" << envelopePath << mismatch;
        }
    }
    qCInfo(CrashReportLog) << "crash reporting armed for" << describeDsn(dsn);
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
    // nothing leaves the machine until giveConsent(). cache_keep holds the previous run's
    // crash envelope in <database>/cache instead of dropping it at init, so the Ask dialog
    // decides its fate: giveConsent() flushes it via the transport retry, revokeConsent()
    // deletes it below. This also covers the first crash after switching to AlwaysSend,
    // which giveConsent() (running after init) would otherwise miss. http_retry is required
    // for the flush: without it the held envelope is written with a plain filename the retry
    // scanner skips, and giveConsent()'s sentry_transport_retry() sends nothing.
    sentry_options_set_require_user_consent(options, 1);
    sentry_options_set_cache_keep(options, SENTRY_CACHE_KEEP_OFFLINE);
    sentry_options_set_http_retry(options, 1);
    sentry_options_set_before_send(options, scrubEventBeforeSend, nullptr);
    ready = (sentry_init(options) == 0);
#else
    Q_UNUSED(release);
    // Not a misconfiguration like an empty DSN: this build cannot report whatever the DSN says, so
    // a tester forcing a crash needs to know the difference.
    qCInfo(CrashReportLog) << "crash reporting unavailable: built without Sentry support";
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

CrashReporter::~CrashReporter() = default;
CrashReporter::CrashReporter() = default;

bool CrashReporter::beginSession(const QString &dir)
{
    // Restartable: releasing first keeps begin->end->begin round-trips clean.
    endSession();

    QDir().mkpath(dir);
    const QDir sessionDir(dir);
    const QString selfName = QStringLiteral("session-%1").arg(QCoreApplication::applicationPid());

    bool unclean = false;
    const QStringList leftovers = sessionDir.entryList({QStringLiteral("session-*")}, QDir::Files);
    for (const QString &name : leftovers) {
        if (name == selfName) {
            continue;
        }
        const QString path = sessionDir.filePath(name);
        QLockFile probe(path);
        // Long-lived session lock: stale means "owner dead", regardless of file age, so a
        // crash followed by an immediate relaunch (younger than the 30 s default) is still
        // detected. tryLock() only deletes the file when it finds it stale, so a live
        // concurrent instance's lock is never touched here. (The static
        // QLockFile::removeStaleLockFile(path) form does not exist in Qt5/Qt6, and the
        // member removeStaleLockFile() force-removes without a staleness check, so neither
        // is usable for this scan.)
        probe.setStaleLockTime(0);
        if (probe.tryLock(0)) {
            unclean = true;
            probe.unlock(); // deletes the stale file
            continue;
        }
        // tryLock failed: either a live concurrent instance owns the file, or it is
        // unreadable (garbage/empty) and therefore has no live owner. Only the latter is
        // cleaned up; a live instance is left alone. On Unix QLockFile creates the file
        // before writing the pid, so a second instance starting at the same moment can
        // briefly observe an empty (or half-written) file for a live owner: re-read once
        // after a beat so a live owner's fresh lock is never mistaken for garbage and deleted.
        qint64 pid = 0;
        QString hostname, appname;
        if (!probe.getLockInfo(&pid, &hostname, &appname)) {
            QThread::msleep(100);
            if (!probe.getLockInfo(&pid, &hostname, &appname)) {
                QFile::remove(path);
                unclean = true;
            }
        }
    }

    sessionLockPath = sessionDir.filePath(selfName);
    sessionLock = std::make_unique<QLockFile>(sessionLockPath);
    sessionLock->setStaleLockTime(0);
    if (!sessionLock->tryLock(0)) {
        qCWarning(CrashReportLog) << "Failed to claim session lock at" << sessionLockPath;
    }
    return unclean;
}

void CrashReporter::endSession()
{
    if (sessionLock) {
        // unlock() removes the lock file; nothing left to clean up.
        sessionLock->unlock();
        sessionLock.reset();
    } else if (!sessionLockPath.isEmpty()) {
        // The lock was never claimed (a live concurrent instance owns its file): only our
        // own unclaimed file, if tryLock left one behind, is removed.
        QFile::remove(sessionLockPath);
    }
    sessionLockPath.clear();
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
        // Drop the held crash envelope: without this it would sit in <database>/cache and
        // upload on the next give (e.g. after switching to AlwaysSend), after the user
        // explicitly refused it. The SDK recreates the directory as needed.
        QDir(sentryDatabasePath + QStringLiteral("/cache")).removeRecursively();
    }
#endif
}

bool CrashReporter::sendReport(const QString &redactedPreview, const QString &userNote)
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
    bool noteAttachOk = true;
    if (!userNote.isEmpty()) {
        const QByteArray noteBytes = userNote.toUtf8();
        const sentry_uuid_t noteId = sentry_scope_attach_bytes(scope, noteBytes.constData(),
                                                               static_cast<size_t>(noteBytes.size()), "user-note.txt");
        noteAttachOk = sentry_uuid_is_nil(&noteId) == 0;
        if (!noteAttachOk) {
            qCWarning(CrashReportLog) << "Could not attach user note to crash report";
        }
    }

    // Release is attached automatically from the init options. Deliberately no
    // sentry_set_user call: the report must carry no PII. The dialog collects
    // no contact info, so the message event + attachments are the whole payload
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
    return attachOk && noteAttachOk;
#else
    Q_UNUSED(redactedPreview);
    Q_UNUSED(userNote);
    return false;
#endif
}

bool CrashReporter::appendAttachmentToEnvelope(const QString &envelopePath,
                                               const QString &filename,
                                               const QByteArray &payload)
{
    // QIODevice::ReadWrite creates a missing file; appending to a file that was never
    // written by the SDK would fabricate an envelope, so require it to exist.
    if (!QFile::exists(envelopePath)) {
        return false;
    }
    QFile file(envelopePath);
    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }
    // Item boundaries are single newlines: make sure the header starts on a fresh line.
    if (file.size() > 0) {
        if (!file.seek(file.size() - 1)) {
            return false;
        }
        char lastByte = 0;
        if (file.read(&lastByte, 1) != 1) {
            return false;
        }
        if (lastByte != '\n' && file.write("\n", 1) != 1) {
            return false;
        }
    }
    // Item shape mirrors sentry__envelope_add_attachment in the SDK. `length` is what the
    // SDK's envelope re-parser uses to frame the payload (a missing length ends the item at
    // the next newline, which would truncate a multi-line log and make the rest of the file
    // fail to parse); `attachment_length` is the logical attachment size the SDK also emits.
    // The filename is a compile-time ASCII constant from the callers, so no JSON escaping is
    // needed.
    const QByteArray header = QByteArrayLiteral("{\"type\":\"attachment\",\"length\":") +
                              QByteArray::number(payload.size()) + QByteArrayLiteral(",\"attachment_length\":") +
                              QByteArray::number(payload.size()) + QByteArrayLiteral(",\"filename\":\"") +
                              filename.toUtf8() + QByteArrayLiteral("\",\"content_type\":\"text/plain\"}") + '\n';
    if (file.write(header) != header.size()) {
        return false;
    }
    return file.write(payload) == payload.size();
}

QStringList CrashReporter::pendingEnvelopePaths(const QString &databasePath)
{
    static const auto isUuid = [](const QString &id) { return id.size() == 36 && id.count(QLatin1Char('-')) == 4; };
    static const auto isEnvelopeName = [](const QString &name) {
        if (name == QStringLiteral("__sentry-crash.envelope")) {
            return false;
        }
        const QString stem = name.left(name.size() - 9); // strip ".envelope"
        if (isUuid(stem)) {
            return true;
        }
        // Retry cache filename: "<ts>-<count>-<uuid>".
        const qsizetype firstSep = stem.indexOf(QLatin1Char('-'));
        const qsizetype secondSep = firstSep > 0 ? stem.indexOf(QLatin1Char('-'), firstSep + 1) : -1;
        return firstSep > 0 && secondSep > 0 && isUuid(stem.mid(secondSep + 1)) && stem.left(firstSep).toLongLong() > 0;
    };
    QStringList result;
    const QDir dbDir(databasePath);
    if (!dbDir.exists()) {
        return result;
    }
    // Consent-held envelopes (Ask mode: the SDK cached them at crash time instead
    // of sending, see sentry__submit_envelope). Bare "<uuid>.envelope" with
    // cache_keep only, "<ts>-<count>-<uuid>.envelope" with http_retry.
    const QDir cacheDir(databasePath + QStringLiteral("/cache"));
    const QStringList cacheNames = cacheDir.entryList({QStringLiteral("*.envelope")}, QDir::Files, QDir::Name);
    for (const QString &name : cacheNames) {
        if (isEnvelopeName(name)) {
            result.append(cacheDir.filePath(name));
        }
    }
    // In-flight envelopes dumped to the crashed run's own directory (AlwaysSend
    // mode: consent was already given, so the transport dump wrote them there and
    // sentry_init's process_old_runs replays them at startup — attach before init).
    const QStringList runDirs =
        dbDir.entryList({QStringLiteral("*.run")}, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &runDir : runDirs) {
        const QDir dir(dbDir.filePath(runDir));
        const QStringList names = dir.entryList({QStringLiteral("*.envelope")}, QDir::Files, QDir::Name);
        for (const QString &name : names) {
            if (isEnvelopeName(name)) {
                result.append(dir.filePath(name));
            }
        }
    }
    return result;
}

int CrashReporter::attachToPendingCrashEnvelopes(const QString &redactedLogText, const QString &userNoteText)
{
    return attachToPendingCrashEnvelopes(sentryDatabasePath, redactedLogText, userNoteText);
}

int CrashReporter::attachToPendingCrashEnvelopes(const QString &databasePath,
                                                 const QString &redactedLogText,
                                                 const QString &userNoteText)
{
    if (databasePath.isEmpty()) {
        return 0;
    }
    const QByteArray logBytes = redactedLogText.toUtf8();
    const QByteArray noteBytes = userNoteText.toUtf8();
    int attached = 0;
    const QStringList envelopes = pendingEnvelopePaths(databasePath);
    for (const QString &path : envelopes) {
        bool ok = appendAttachmentToEnvelope(path, QStringLiteral("crash-log.txt"), logBytes);
        if (ok && !noteBytes.isEmpty()) {
            ok = appendAttachmentToEnvelope(path, QStringLiteral("user-note.txt"), noteBytes);
        }
        if (ok) {
            ++attached;
        } else {
            qCWarning(CrashReportLog) << "Failed to attach crash log to pending envelope" << path;
        }
    }
    if (attached > 0) {
        qCInfo(CrashReportLog) << "Attached crash log and user note to" << attached << "pending crash envelope(s)";
    }
    return attached;
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
