#include "smtpclient.h"

#include "settingscache.h"
#include "smtp/qxtsmtp.h"

#include <QSslSocket>
#include <QTcpSocket>

SmtpClient::SmtpClient(QObject *parent) : QObject(parent)
{
    smtp = new QxtSmtp(this);

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
        smtp = 0;
    }
}

bool SmtpClient::enqueueActivationTokenMail(const QString &nickname, const QString &recipient, const QString &token)
{
    QString email = settingsCache->value("smtp/email", "").toString();
    QString name = settingsCache->value("smtp/name", "").toString();
    QString subject = settingsCache->value("smtp/subject", "").toString();
    QString body = settingsCache->value("smtp/body", "").toString();

    if (email.isEmpty()) {
        qDebug() << "[MAIL] Missing sender email in configuration";
        return false;
    }

    if (subject.isEmpty()) {
        qDebug() << "[MAIL] Missing subject field in configuration";
        return false;
    }

    if (body.isEmpty()) {
        qDebug() << "[MAIL] Missing body field in configuration";
        return false;
    }

    if (recipient.isEmpty()) {
        qDebug() << "[MAIL] Missing recipient field for user " << nickname;
        return false;
    }

    if (token.isEmpty()) {
        qDebug() << "[MAIL] Missing token field for user " << nickname;
        return false;
    }

    QxtMailMessage message;
    message.setSender(name + " <" + email + ">");
    message.addRecipient(recipient);
    message.setSubject(subject);
    message.setBody(body.replace("%username", nickname).replace("%token", token));

    int id = smtp->send(message);
    qDebug() << "[MAIL] Enqueued mail to" << recipient << "as" << id;
    return true;
}

bool SmtpClient::enqueueForgotPasswordTokenMail(const QString &nickname, const QString &recipient, const QString &token)
{
    QString email = settingsCache->value("smtp/email", "").toString();
    QString name = settingsCache->value("smtp/name", "").toString();
    QString subject = settingsCache->value("forgotpassword/subject", "").toString();
    QString body = settingsCache->value("forgotpassword/body", "").toString();

    if (email.isEmpty()) {
        qDebug() << "[MAIL] Missing sender email in configuration";
        return false;
    }

    if (subject.isEmpty()) {
        qDebug() << "[MAIL] Missing subject field in configuration";
        return false;
    }

    if (body.isEmpty()) {
        qDebug() << "[MAIL] Missing body field in configuration";
        return false;
    }

    if (recipient.isEmpty()) {
        qDebug() << "[MAIL] Missing recipient field for user " << nickname;
        return false;
    }

    if (token.isEmpty()) {
        qDebug() << "[MAIL] Missing token field for user " << nickname;
        return false;
    }

    QxtMailMessage message;
    message.setSender(name + " <" + email + ">");
    message.addRecipient(recipient);
    message.setSubject(subject);
    message.setBody(body.replace("%username", nickname).replace("%token", token));

    int id = smtp->send(message);
    qDebug() << "[MAIL] Enqueued mail to" << recipient << "as" << id;
    return true;
}

void SmtpClient::sendAllEmails()
{
    // still connected from the previous round
    if (smtp->socket()->state() == QAbstractSocket::ConnectedState)
        return;

    if (smtp->pendingMessages() == 0)
        return;

    QString connectionType = settingsCache->value("smtp/connection", "tcp").toString();
    QString host = settingsCache->value("smtp/host", "localhost").toString();
    int port = settingsCache->value("smtp/port", 25).toInt();
    QByteArray username = settingsCache->value("smtp/username", "").toByteArray();
    QByteArray password = settingsCache->value("smtp/password", "").toByteArray();
    bool acceptAllCerts = settingsCache->value("smtp/acceptallcerts", false).toBool();

    smtp->setUsername(username);
    smtp->setPassword(password);

    // Connect
    if (connectionType == "ssl") {
        if (acceptAllCerts)
            smtp->sslSocket()->setPeerVerifyMode(QSslSocket::QueryPeer);
        smtp->connectToSecureHost(host, port);
    } else {
        smtp->connectToHost(host, port);
    }
}

void SmtpClient::authenticated()
{
    qDebug() << "[MAIL] authenticated";
}

void SmtpClient::authenticationFailed(const QByteArray &msg)
{
    qDebug() << "[MAIL] authenticationFailed" << QString(msg);
}

void SmtpClient::connected()
{
    qDebug() << "[MAIL] connected";
}

void SmtpClient::connectionFailed(const QByteArray &msg)
{
    qDebug() << "[MAIL] connectionFailed" << QString(msg);
}

void SmtpClient::disconnected()
{
    qDebug() << "[MAIL] disconnected";
}

void SmtpClient::encrypted()
{
    qDebug() << "[MAIL] encrypted";
}

void SmtpClient::encryptionFailed(const QByteArray &msg)
{
    qDebug() << "[MAIL] encryptionFailed" << QString(msg);
    qDebug() << "[MAIL] Try enabling the \"acceptallcerts\" option in servatrice.ini";
}

void SmtpClient::finished()
{
    qDebug() << "[MAIL] finished";
    smtp->disconnectFromHost();
}

void SmtpClient::mailFailed(int mailID, int errorCode, const QByteArray &msg)
{
    qDebug() << "[MAIL] mailFailed id=" << mailID << " errorCode=" << errorCode << "msg=" << QString(msg);
}

void SmtpClient::mailSent(int mailID)
{
    qDebug() << "[MAIL] mailSent" << mailID;
}

void SmtpClient::recipientRejected(int mailID, const QString &address, const QByteArray &msg)
{
    qDebug() << "[MAIL] recipientRejected id=" << mailID << " address=" << address << "msg=" << QString(msg);
}

void SmtpClient::senderRejected(int mailID, const QString &address, const QByteArray &msg)
{
    qDebug() << "[MAIL] senderRejected id=" << mailID << " address=" << address << "msg=" << QString(msg);
}
