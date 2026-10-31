#include "smtpclient.h"

#include "settingscache.h"
#include "smtp/qxtmailmessage.h"
#include "smtp/qxtsmtp.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QDebug>
#include <QLoggingCategory>
#include <QSharedPointer>
#include <QSslSocket>
#include <QString>
#include <QTcpSocket>
#include <QVariant>
#include <qlogging.h>
#include <qobjectdefs.h>

static const qint64 DEFAULT_MAX_RETRIES = 5;
static const qint64 DEFAULT_RETRY_DELAY = 60;
static const qint64 MAX_RETRY_DELAY = 1800;  // 30 minutes
static const qint64 IDLE_TIMEOUT_MS = 60000; // 60 seconds
// Maximum bit-shift exponent used in the exponential back-off formula.
// With a base delay of 60 s, shift 10 gives 60 * 1024 ≈ 17 h before the
// MAX_RETRY_DELAY cap is applied, preventing unbounded intermediate values.
static const int MAX_BACKOFF_SHIFT = 10;

inline Q_LOGGING_CATEGORY(SmtpClientLog, "smtp_client")

static QString pendingKey(const QString &userName, EmailType type, const QString &token)
{
    const char *typeName = (type == EmailType::Activation) ? "activation" : "forgotpassword";
    // The token is part of the key: a regenerated token (forgot-password issues a
    // fresh one per request) must not be suppressed by a stale pending entry.
    return userName + '/' + typeName + '/' + token;
}

// True only for permanent 5xx replies. Anything else (4xx greylisting/rate
// limiting, or an unparsable reply) must be retried, never treated as final:
// destroying the durable queue row on a transient failure loses the mail.
static bool isPermanentSmtpReply(const QByteArray &msg)
{
    for (char c : msg) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '-') {
            continue;
        }
        return c == '5';
    }
    return false;
}

SmtpClient::SmtpClient(QObject *parent) : QObject(parent)
{
    smtp = new QxtSmtp(this);

    idleTimer = new QTimer(this);
    idleTimer->setSingleShot(true);
    connect(idleTimer, SIGNAL(timeout()), this, SLOT(idleTimeout()));

    connect(smtp, SIGNAL(authenticated()), this, SLOT(authenticated()));
    connect(smtp, SIGNAL(authenticationFailed(const QByteArray &)), this,
            SLOT(authenticationFailed(const QByteArray &)));
    connect(smtp, SIGNAL(connected()), this, SLOT(connected()));
    connect(smtp, SIGNAL(connectionFailed(const QByteArray &)), this, SLOT(connectionFailed(const QByteArray &)));
    connect(smtp, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(smtp, SIGNAL(encrypted()), this, SLOT(encrypted()));
    connect(smtp, SIGNAL(encryptionFailed(const QByteArray &)), this, SLOT(encryptionFailed(const QByteArray &)));
    connect(smtp, SIGNAL(finished()), this, SLOT(finished()));
    connect(smtp, SIGNAL(mailFailed(int, int, const QByteArray &)), this,
            SLOT(mailFailed(int, int, const QByteArray &)));
    connect(smtp, SIGNAL(mailSent(int)), this, SLOT(mailSent(int)));
    connect(smtp, SIGNAL(recipientRejected(int, const QString &, const QByteArray &)), this,
            SLOT(recipientRejected(int, const QString &, const QByteArray &)));
    connect(smtp, SIGNAL(senderRejected(int, const QString &, const QByteArray &)), this,
            SLOT(senderRejected(int, const QString &, const QByteArray &)));
}

SmtpClient::~SmtpClient()
{
    if (smtp) {
        delete smtp;
        smtp = nullptr;
    }
}

int SmtpClient::maxRetries() const
{
    // Bounded like retryDelaySeconds(): an unset/blank value must not turn the
    // first transient failure into a permanent one (attempts >= 0 is immediate).
    return qBound(1, settingsCache->value("smtp/maxretries", DEFAULT_MAX_RETRIES).toInt(), 100);
}

int SmtpClient::retryDelaySeconds() const
{
    return qBound(1, settingsCache->value("smtp/retrydelay", DEFAULT_RETRY_DELAY).toInt(),
                  static_cast<int>(MAX_RETRY_DELAY));
}

QDateTime SmtpClient::nextRetryTime(int attempts, int retryDelay) const
{
    // Exponential backoff, capped to avoid hammering an unreachable server.
    const int shift = qMax(0, qMin(attempts - 1, MAX_BACKOFF_SHIFT));
    const qint64 delay = qMin<qint64>(retryDelay * qint64(1) << shift, MAX_RETRY_DELAY);
    return QDateTime::currentDateTime().addSecs(delay);
}

SmtpClient::PendingEmail SmtpClient::buildEmail(const QString &nickname,
                                                EmailType type,
                                                const QString &subject,
                                                const QString &body,
                                                const QString &recipient,
                                                const QString &token,
                                                const QString &email,
                                                qint64 rowId)
{
    QString name = settingsCache->value("smtp/name", "").toString();

    PendingEmail pending;
    pending.userName = nickname;
    pending.type = type;
    pending.token = token;
    pending.rowId = rowId;
    pending.maxAttempts = maxRetries();
    pending.retryDelay = retryDelaySeconds();
    pending.nextAttempt = QDateTime::currentDateTime();

    QString emailBody = body;
    emailBody.replace("%username", nickname).replace("%token", token);

    pending.message.setSender(name + " <" + email + ">");
    pending.message.addRecipient(recipient);
    pending.message.setSubject(subject);
    pending.message.setBody(emailBody);

    return pending;
}

bool SmtpClient::enqueue(const PendingEmail &email)
{
    const QString key = pendingKey(email.userName, email.type, email.token);
    if (pendingEmails.contains(key)) {
        qCDebug(SmtpClientLog) << "Email already pending for" << email.userName << "- not duplicating";
        return false;
    }

    // A fresher database row for the same user/type supersedes a stale pending
    // entry: drop the stale one so only the current token is ever delivered.
    // (Any in-flight mapping for the dropped key is removed as well; the
    // mailSent/recordFailed lookups tolerate missing entries.)
    for (auto it = pendingEmails.begin(); it != pendingEmails.end();) {
        if (it.value().userName == email.userName && it.value().type == email.type) {
            for (auto mapIt = mailIdToUser.begin(); mapIt != mailIdToUser.end();) {
                if (mapIt.value() == it.key()) {
                    mapIt = mailIdToUser.erase(mapIt);
                } else {
                    ++mapIt;
                }
            }
            qCDebug(SmtpClientLog) << "Replacing stale pending email for" << email.userName;
            it = pendingEmails.erase(it);
        } else {
            ++it;
        }
    }

    pendingEmails.insert(key, email);
    qCDebug(SmtpClientLog) << "Enqueued" << (email.type == EmailType::Activation ? "activation" : "password reset")
                           << "mail to" << email.userName;
    return true;
}

bool SmtpClient::enqueueActivationTokenMail(const QString &nickname, const QString &recipient, const QString &token)
{
    if (pendingEmails.contains(pendingKey(nickname, EmailType::Activation, token))) {
        qCDebug(SmtpClientLog) << "Activation email already pending for" << nickname << "- skipping";
        return false;
    }

    QString subject = settingsCache->value("smtp/subject", "").toString();
    QString body = settingsCache->value("smtp/body", "").toString();
    QString email = settingsCache->value("smtp/email", "").toString();

    if (!validateEmailFields(email, subject, body, recipient, token, nickname)) {
        return false;
    }

    return enqueue(buildEmail(nickname, EmailType::Activation, subject, body, recipient, token, email));
}

bool SmtpClient::enqueueForgotPasswordTokenMail(const QString &nickname,
                                                const QString &recipient,
                                                const QString &token,
                                                qint64 rowId)
{
    if (pendingEmails.contains(pendingKey(nickname, EmailType::ForgotPassword, token))) {
        qCDebug(SmtpClientLog) << "Password reset email already pending for" << nickname << "- skipping";
        return false;
    }

    QString subject = settingsCache->value("forgotpassword/subject", "").toString();
    QString body = settingsCache->value("forgotpassword/body", "").toString();
    QString email = settingsCache->value("smtp/email", "").toString();

    if (!validateEmailFields(email, subject, body, recipient, token, nickname)) {
        return false;
    }

    return enqueue(buildEmail(nickname, EmailType::ForgotPassword, subject, body, recipient, token, email, rowId));
}

void SmtpClient::sendAllEmails()
{
    // still connected from the previous round
    if (smtp->socket()->state() == QAbstractSocket::ConnectedState ||
        smtp->socket()->state() == QAbstractSocket::ConnectingState ||
        smtp->socket()->state() == QAbstractSocket::HostLookupState ||
        smtp->socket()->state() == QAbstractSocket::ClosingState) {
        return;
    }

    if (pendingEmails.isEmpty()) {
        return;
    }

    // Respect connection-level backoff after auth/conn/encryption failures.
    if (connectionAttempts > 0 && QDateTime::currentDateTime() < connectionBackoffUntil) {
        return;
    }

    QString connectionType = settingsCache->value("smtp/connection", "tcp").toString();
    QString host = settingsCache->value("smtp/host", "localhost").toString();
    int port = settingsCache->value("smtp/port", 25).toInt();
    QByteArray username = settingsCache->value("smtp/username", "").toByteArray();
    QByteArray password = settingsCache->value("smtp/password", "").toByteArray();
    bool acceptAllCerts = settingsCache->value("smtp/acceptallcerts", false).toBool();

    smtp->setUsername(username);
    smtp->setPassword(password);

    // Push due emails into QxtSmtp *before* connecting so they are already
    // queued when the authenticated() signal fires and QxtSmtp's internal
    // sendNext() runs.
    relayoutForSending();

    // Nothing due right now — skip the SMTP round-trip.
    if (mailIdToUser.isEmpty()) {
        return;
    }

    // Connect
    if (connectionType == "ssl") {
        if (acceptAllCerts) {
            smtp->sslSocket()->setPeerVerifyMode(QSslSocket::QueryPeer);
        }
        smtp->connectToSecureHost(host, port);
    } else {
        smtp->connectToHost(host, port);
    }
}

void SmtpClient::relayoutForSending()
{
    // Rebuild the SMTP buffer exclusively from our authoritative set, so a reconnect
    // after a partial transmission never results in duplicate deliveries.
    // reset() also clears the internal state machine so that send() does not
    // fire sendNext() prematurely on a not-yet-connected socket.
    smtp->reset();
    mailIdToUser.clear();

    const QDateTime now = QDateTime::currentDateTime();
    for (auto it = pendingEmails.begin(); it != pendingEmails.end(); ++it) {
        PendingEmail &pending = it.value();
        if (pending.nextAttempt > now) {
            continue; // not yet due; honours the backoff schedule
        }
        int id = smtp->send(pending.message);
        mailIdToUser.insert(id, pendingKey(pending.userName, pending.type, pending.token));
    }
}

void SmtpClient::finished()
{
    qCDebug(SmtpClientLog) << "finished";
    smtp->disconnectFromHost();
}

void SmtpClient::authenticated()
{
    qCDebug(SmtpClientLog) << "authenticated";
    // The session has actually succeeded (TCP connect alone proves nothing:
    // TLS/auth may still fail), so the connection-level backoff resets here.
    connectionAttempts = 0;
    // Start the idle timer here, once the connection is fully ready to send,
    // rather than on TCP connect: SSL/STARTTLS handshake and auth may take a
    // while and must not be cut short by the idle timeout.
    idleTimer->start(IDLE_TIMEOUT_MS);
}

void SmtpClient::authenticationFailed(const QByteArray &msg)
{
    qCWarning(SmtpClientLog) << "authenticationFailed" << QString::fromUtf8(msg);
    idleTimer->stop();
    recordConnectionFailure();
    mailIdToUser.clear();
    smtp->disconnectFromHost();
}

void SmtpClient::connected()
{
    qCDebug(SmtpClientLog) << "connected";
    // NOTE: connectionAttempts is deliberately not reset here. This fires on
    // TCP connect, before the TLS handshake and AUTH; resetting here would pin
    // the connection-level backoff at its base delay for exactly the failures
    // (bad password/cert/STARTTLS) it is meant to cover. It resets in
    // authenticated() and on the first successful mailSent instead.
}

void SmtpClient::connectionFailed(const QByteArray &msg)
{
    qCWarning(SmtpClientLog) << "connectionFailed" << QString::fromUtf8(msg);
    idleTimer->stop();
    recordConnectionFailure();
    mailIdToUser.clear();
    smtp->disconnectFromHost();
}

void SmtpClient::disconnected()
{
    qCDebug(SmtpClientLog) << "disconnected - emails still pending:" << pendingEmails.count();
    idleTimer->stop();
    // A mid-session drop (peer close/RST past the connect phase, or the idle
    // timeout firing mid-batch) emits no mailFailed/connectionFailed signal, so
    // route every in-flight mail through recordFailed(): otherwise attempts and
    // backoff are bypassed and the mails requeue with stale state forever.
    const QList<int> inFlight = mailIdToUser.keys();
    for (int mailID : inFlight) {
        recordFailed(mailID);
    }
    mailIdToUser.clear();
}

void SmtpClient::encrypted()
{
    qCDebug(SmtpClientLog) << "encrypted";
}

void SmtpClient::encryptionFailed(const QByteArray &msg)
{
    qCWarning(SmtpClientLog) << "encryptionFailed" << QString::fromUtf8(msg);
    qCWarning(SmtpClientLog) << "Try enabling the \"acceptallcerts\" option in servatrice.ini";
    idleTimer->stop();
    recordConnectionFailure();
    mailIdToUser.clear();
    smtp->disconnectFromHost();
}

void SmtpClient::idleTimeout()
{
    qCDebug(SmtpClientLog) << "idle timeout - disconnecting";
    smtp->disconnectFromHost();
}

void SmtpClient::recordFailed(int mailID)
{
    auto it = mailIdToUser.find(mailID);
    if (it == mailIdToUser.end()) {
        return;
    }
    const QString key = it.value();
    mailIdToUser.erase(it);

    auto pendingIt = pendingEmails.find(key);
    if (pendingIt == pendingEmails.end()) {
        return;
    }

    PendingEmail &pending = pendingIt.value();
    pending.attempts++;

    if (pending.attempts >= pending.maxAttempts) {
        const QString userName = pending.userName;
        const EmailType type = pending.type;
        const QString token = pending.token;
        const qint64 rowId = pending.rowId;
        qCWarning(SmtpClientLog) << "Email to" << userName << "permanently failed after" << pending.attempts
                                 << "attempts - dropping from pending set";
        pendingEmails.erase(pendingIt);
        emit mailPermanentlyFailed(userName, type, FailureReason::RetryExhausted, token, rowId);
        return;
    }

    pending.nextAttempt = nextRetryTime(pending.attempts, pending.retryDelay);
    qCWarning(SmtpClientLog) << "Email to" << pending.userName << "failed (attempt" << pending.attempts << "of"
                             << pending.maxAttempts << "), retrying" << pending.nextAttempt.toString(Qt::ISODate);
}

void SmtpClient::recordConnectionFailure()
{
    ++connectionAttempts;
    connectionBackoffUntil = nextRetryTime(connectionAttempts, retryDelaySeconds());
    qCWarning(SmtpClientLog) << "Connection-layer failure #" << connectionAttempts << "- next attempt after"
                             << connectionBackoffUntil.toString(Qt::ISODate);

    // Connection failures count toward the per-email retry budget so that
    // undeliverable mail eventually gives up rather than retrying forever.
    QList<QString> keysToRemove;
    for (auto it = pendingEmails.begin(); it != pendingEmails.end(); ++it) {
        PendingEmail &pending = it.value();
        pending.attempts++;
        if (pending.attempts >= pending.maxAttempts) {
            qCWarning(SmtpClientLog) << "Email to" << pending.userName << "permanently failed after" << pending.attempts
                                     << "connection failures - dropping from pending set";
            keysToRemove.append(it.key());
            emit mailPermanentlyFailed(pending.userName, pending.type, FailureReason::RetryExhausted, pending.token,
                                       pending.rowId);
        }
    }
    for (const QString &key : keysToRemove) {
        pendingEmails.remove(key);
    }
}

bool SmtpClient::validateEmailFields(const QString &email,
                                     const QString &subject,
                                     const QString &body,
                                     const QString &recipient,
                                     const QString &token,
                                     const QString &nickname)
{
    if (email.isEmpty()) {
        qCCritical(SmtpClientLog) << "Missing sender email in configuration";
        return false;
    }
    if (subject.isEmpty()) {
        qCCritical(SmtpClientLog) << "Missing subject field in configuration";
        return false;
    }
    if (body.isEmpty()) {
        qCCritical(SmtpClientLog) << "Missing body field in configuration";
        return false;
    }
    if (recipient.isEmpty()) {
        qCCritical(SmtpClientLog) << "Missing recipient field for user" << nickname;
        return false;
    }
    if (token.isEmpty()) {
        qCCritical(SmtpClientLog) << "Missing token field for user" << nickname;
        return false;
    }
    return true;
}

void SmtpClient::mailFailed(int mailID, int errorCode, const QByteArray &msg)
{
    qCWarning(SmtpClientLog) << "mailFailed id=" << mailID << " errorCode=" << errorCode
                             << "msg=" << QString::fromUtf8(msg);
    // Progress was made (the server answered), so the idle timer measures from
    // here rather than acting as a cap on the whole session.
    idleTimer->start(IDLE_TIMEOUT_MS);
    recordFailed(mailID);
}

void SmtpClient::mailSent(int mailID)
{
    qCDebug(SmtpClientLog) << "mailSent" << mailID;
    // A delivered mail proves the session works: reset the connection-level
    // backoff and restart the idle timer from this point of progress.
    connectionAttempts = 0;
    idleTimer->start(IDLE_TIMEOUT_MS);

    auto it = mailIdToUser.find(mailID);
    if (it == mailIdToUser.end()) {
        return;
    }
    const QString key = it.value();
    mailIdToUser.erase(it);

    auto pendingIt = pendingEmails.find(key);
    if (pendingIt == pendingEmails.end()) {
        return;
    }

    const QString userName = pendingIt.value().userName;
    const EmailType type = pendingIt.value().type;
    const QString token = pendingIt.value().token;
    const qint64 rowId = pendingIt.value().rowId;
    pendingEmails.erase(pendingIt);
    qCDebug(SmtpClientLog) << "Delivered email to" << userName;
    emit mailDelivered(userName, type, token, rowId);
}

void SmtpClient::recipientRejected(int mailID, const QString &address, const QByteArray &msg)
{
    qCWarning(SmtpClientLog) << "recipientRejected id=" << mailID << " address=" << address
                             << "msg=" << QString::fromUtf8(msg);
    if (!isPermanentSmtpReply(msg)) {
        // Transient 4xx (e.g. greylisting): consume one attempt and reschedule
        // with backoff instead of destroying the durable queue row.
        recordFailed(mailID);
        return;
    }
    // A rejected recipient is a permanent condition; consider the mail undeliverable.
    auto it = mailIdToUser.find(mailID);
    if (it != mailIdToUser.end()) {
        const QString key = it.value();
        if (auto pendingIt = pendingEmails.find(key); pendingIt != pendingEmails.end()) {
            const QString userName = pendingIt.value().userName;
            const EmailType type = pendingIt.value().type;
            const QString token = pendingIt.value().token;
            const qint64 rowId = pendingIt.value().rowId;
            pendingEmails.erase(pendingIt);
            qCWarning(SmtpClientLog) << "Email to" << userName << "rejected - dropping from pending set";
            emit mailPermanentlyFailed(userName, type, FailureReason::RecipientRejected, token, rowId);
        }
        mailIdToUser.erase(it);
    }
}

void SmtpClient::senderRejected(int mailID, const QString &address, const QByteArray &msg)
{
    qCWarning(SmtpClientLog) << "senderRejected id=" << mailID << " sender=" << address
                             << "msg=" << QString::fromUtf8(msg);
    if (!isPermanentSmtpReply(msg)) {
        // Transient 4xx on MAIL FROM (e.g. rate limiting): retry with backoff.
        recordFailed(mailID);
        return;
    }
    // A rejected sender is a permanent condition; consider the mail undeliverable.
    auto it = mailIdToUser.find(mailID);
    if (it != mailIdToUser.end()) {
        const QString key = it.value();
        if (auto pendingIt = pendingEmails.find(key); pendingIt != pendingEmails.end()) {
            const QString userName = pendingIt.value().userName;
            const EmailType type = pendingIt.value().type;
            const QString token = pendingIt.value().token;
            const qint64 rowId = pendingIt.value().rowId;
            pendingEmails.erase(pendingIt);
            qCWarning(SmtpClientLog) << "Email to" << userName << "sender rejected - dropping from pending set";
            emit mailPermanentlyFailed(userName, type, FailureReason::SenderRejected, token, rowId);
        }
        mailIdToUser.erase(it);
    }
}
