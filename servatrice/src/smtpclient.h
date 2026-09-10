#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include "smtp/qxtmailmessage.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QTimer>

class QxtSmtp;

/**
 * @brief Types of e-mail the SMTP client is able to deliver.
 */
enum class EmailType
{
    Activation,
    ForgotPassword
};

Q_DECLARE_METATYPE(EmailType)

/**
 * @brief Why a permanent e-mail delivery failure occurred.
 */
enum class FailureReason
{
    RetryExhausted,
    RecipientRejected,
    SenderRejected
};

Q_DECLARE_METATYPE(FailureReason)

class SmtpClient : public QObject
{
    Q_OBJECT
public:
    SmtpClient(QObject *parent = nullptr);
    ~SmtpClient() override;

public slots:
    bool enqueueActivationTokenMail(const QString &nickname, const QString &recipient, const QString &token);
    bool enqueueForgotPasswordTokenMail(const QString &nickname, const QString &recipient, const QString &token);
    void sendAllEmails();

signals:
    void mailDelivered(const QString &userName, EmailType type);
    void mailPermanentlyFailed(const QString &userName, EmailType type, FailureReason reason);

protected:
    QxtSmtp *smtp;

protected slots:
    void authenticated();
    void authenticationFailed(const QByteArray &msg);
    void connected();
    void connectionFailed(const QByteArray &msg);
    void disconnected();
    void encrypted();
    void encryptionFailed(const QByteArray &msg);
    void finished();
    void idleTimeout();
    void mailFailed(int mailID, int errorCode, const QByteArray &msg);
    void mailSent(int mailID);
    void recipientRejected(int mailID, const QString &address, const QByteArray &msg);
    void senderRejected(int mailID, const QString &address, const QByteArray &msg);

private:
    struct PendingEmail
    {
        QString userName;
        EmailType type;
        QxtMailMessage message;
        int attempts = 0;
        int maxAttempts = 0;
        int retryDelay = 0;
        QDateTime nextAttempt;
    };

    PendingEmail buildEmail(const QString &nickname,
                            EmailType type,
                            const QString &subject,
                            const QString &body,
                            const QString &recipient,
                            const QString &token,
                            const QString &email);
    bool enqueue(const PendingEmail &email);
    void relayoutForSending();
    QDateTime nextRetryTime(int attempts, int retryDelay) const;
    void recordFailed(int mailID);
    void recordConnectionFailure();
    bool validateEmailFields(const QString &email,
                             const QString &subject,
                             const QString &body,
                             const QString &recipient,
                             const QString &token,
                             const QString &nickname);
    int maxRetries() const;
    int retryDelaySeconds() const;

    QHash<QString, PendingEmail> pendingEmails;
    QHash<int, QString> mailIdToUser;
    QTimer *idleTimer;
    int connectionAttempts = 0;
    QDateTime connectionBackoffUntil;
};

#endif
