/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtWeb module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

/*!
 * \class QxtSmtp
 * \inmodule QxtNetwork
 * \brief The QxtSmtp class implements the SMTP protocol for sending email
 */

#include "qxtsmtp.h"

#include "qxthmac.h"
#include "qxtsmtp_p.h"

#include <QNetworkInterface>
#include <QSslSocket>
#include <QStringList>
#include <QTcpSocket>

QxtSmtpPrivate::QxtSmtpPrivate() : QObject(0)
{
    // empty ctor
}

QxtSmtp::QxtSmtp(QObject *parent) : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtSmtp);
    qxt_d().state = QxtSmtpPrivate::Disconnected;
    qxt_d().nextID = 0;
    qxt_d().socket = new QSslSocket(this);
    QObject::connect(socket(), SIGNAL(encrypted()), this, SIGNAL(encrypted()));
    // QObject::connect(socket(), SIGNAL(encrypted()), &qxt_d(), SLOT(ehlo()));
    QObject::connect(socket(), SIGNAL(connected()), this, SIGNAL(connected()));
    QObject::connect(socket(), SIGNAL(disconnected()), this, SIGNAL(disconnected()));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QObject::connect(socket(), SIGNAL(errorOccurred(QAbstractSocket::SocketError)), &qxt_d(),
                     SLOT(socketError(QAbstractSocket::SocketError)));
#else
    QObject::connect(socket(), SIGNAL(error(QAbstractSocket::SocketError)), &qxt_d(),
                     SLOT(socketError(QAbstractSocket::SocketError)));
#endif
    QObject::connect(this, SIGNAL(authenticated()), &qxt_d(), SLOT(sendNext()));
    QObject::connect(socket(), SIGNAL(readyRead()), &qxt_d(), SLOT(socketRead()));
}

QByteArray QxtSmtp::username() const
{
    return qxt_d().username;
}

void QxtSmtp::setUsername(const QByteArray &username)
{
    qxt_d().username = username;
}

QByteArray QxtSmtp::password() const
{
    return qxt_d().password;
}

void QxtSmtp::setPassword(const QByteArray &password)
{
    qxt_d().password = password;
}

int QxtSmtp::send(const QxtMailMessage &message)
{
    int messageID = ++qxt_d().nextID;
    qxt_d().pending.append(qMakePair(messageID, message));
    if (qxt_d().state == QxtSmtpPrivate::Waiting)
        qxt_d().sendNext();
    return messageID;
}

int QxtSmtp::pendingMessages() const
{
    return qxt_d().pending.count();
}

QTcpSocket *QxtSmtp::socket() const
{
    return qxt_d().socket;
}

void QxtSmtp::connectToHost(const QString &hostName, quint16 port)
{
    qxt_d().useSecure = false;
    qxt_d().state = QxtSmtpPrivate::StartState;
    socket()->connectToHost(hostName, port);
}

void QxtSmtp::connectToHost(const QHostAddress &address, quint16 port)
{
    connectToHost(address.toString(), port);
}

void QxtSmtp::disconnectFromHost()
{
    socket()->disconnectFromHost();
}

bool QxtSmtp::startTlsDisabled() const
{
    return qxt_d().disableStartTLS;
}

void QxtSmtp::setStartTlsDisabled(bool disable)
{
    qxt_d().disableStartTLS = disable;
}

QSslSocket *QxtSmtp::sslSocket() const
{
    return qxt_d().socket;
}

void QxtSmtp::connectToSecureHost(const QString &hostName, quint16 port)
{
    qxt_d().useSecure = true;
    qxt_d().state = QxtSmtpPrivate::StartState;
    sslSocket()->connectToHostEncrypted(hostName, port);
}

void QxtSmtp::connectToSecureHost(const QHostAddress &address, quint16 port)
{
    connectToSecureHost(address.toString(), port);
}

bool QxtSmtp::hasExtension(const QString &extension)
{
    return qxt_d().extensions.contains(extension);
}

QString QxtSmtp::extensionData(const QString &extension)
{
    return qxt_d().extensions[extension];
}

void QxtSmtpPrivate::socketError(QAbstractSocket::SocketError err)
{
    if (err == QAbstractSocket::SslHandshakeFailedError) {
        emit qxt_p().encryptionFailed();
        emit qxt_p().encryptionFailed(socket->errorString().toLatin1());
    } else if (state == StartState) {
        emit qxt_p().connectionFailed();
        emit qxt_p().connectionFailed(socket->errorString().toLatin1());
    }
}

void QxtSmtpPrivate::socketRead()
{
    buffer += socket->readAll();
    while (true) {
        int pos = buffer.indexOf("\r\n");
        if (pos < 0)
            return;
        QByteArray line = buffer.left(pos);
        buffer = buffer.mid(pos + 2);
        QByteArray code = line.left(3);
        switch (state) {
            case StartState:
                if (code[0] != '2') {
                    socket->disconnectFromHost();
                } else {
                    ehlo();
                }
                break;
            case HeloSent:
            case EhloSent:
            case EhloGreetReceived:
                parseEhlo(code, (line[3] != ' '), line.mid(4));
                break;
            case StartTLSSent:
                if (code == "220") {
                    socket->startClientEncryption();
                    ehlo();
                } else {
                    authenticate();
                }
                break;
            case AuthRequestSent:
            case AuthUsernameSent:
                if (authType == AuthPlain)
                    authPlain();
                else if (authType == AuthLogin)
                    authLogin();
                else
                    authCramMD5(line.mid(4));
                break;
            case AuthSent:
                if (code[0] == '2') {
                    state = Authenticated;
                    emit qxt_p().authenticated();
                } else {
                    state = Disconnected;
                    emit qxt_p().authenticationFailed();
                    emit qxt_p().authenticationFailed(line);
                    emit socket->disconnectFromHost();
                }
                break;
            case MailToSent:
            case RcptAckPending:
                if (code[0] != '2') {
                    emit qxt_p().mailFailed(pending.first().first, code.toInt());
                    emit qxt_p().mailFailed(pending.first().first, code.toInt(), line);
                    // pending.removeFirst();
                    // DO NOT remove it, the body sent state needs this message to assigned the next mail failed message
                    // that will the sendNext a reset will be sent to clear things out
                    sendNext();
                    state = BodySent;
                } else
                    sendNextRcpt(code, line);
                break;
            case SendingBody:
                sendBody(code, line);
                break;
            case BodySent:
                if (pending.count()) {
                    // if you removeFirst in RcpActpending/MailToSent on an error, and the queue is now empty,
                    // you will get into this state and then crash because no check is done.  CHeck added but shouldnt
                    // be necessary since I commented out the removeFirst
                    if (code[0] != '2') {
                        emit qxt_p().mailFailed(pending.first().first, code.toInt());
                        emit qxt_p().mailFailed(pending.first().first, code.toInt(), line);
                    } else
                        emit qxt_p().mailSent(pending.first().first);
                    pending.removeFirst();
                }
                sendNext();
                break;
            case Resetting:
                if (code[0] != '2') {
                    emit qxt_p().connectionFailed();
                    emit qxt_p().connectionFailed(line);
                } else {
                    state = Waiting;
                    sendNext();
                }
                break;
            case Disconnected:
            case EhloExtensionsReceived:
            case EhloDone:
            case Authenticated:
            case Waiting:
                // only to make compiler happy
                break;
        }
    }
}

void QxtSmtpPrivate::ehlo()
{
    QByteArray address = "127.0.0.1";
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (addr == QHostAddress::LocalHost || addr == QHostAddress::LocalHostIPv6)
            continue;
        address = addr.toString().toLatin1();
        break;
    }
    socket->write("ehlo " + address + "\r\n");
    extensions.clear();
    state = EhloSent;
}

void QxtSmtpPrivate::parseEhlo(const QByteArray &code, bool cont, const QString &line)
{
    if (code != "250") {
        // error!
        if (state != HeloSent) {
            // maybe let's try HELO
            socket->write("helo\r\n");
            state = HeloSent;
        } else {
            // nope
            socket->write("QUIT\r\n");
            socket->flush();
            socket->disconnectFromHost();
        }
        return;
    } else if (state != EhloGreetReceived) {
        if (!cont) {
            // greeting only, no extensions
            state = EhloDone;
        } else {
            // greeting followed by extensions
            state = EhloGreetReceived;
            return;
        }
    } else {
        extensions[line.section(' ', 0, 0).toUpper()] = line.section(' ', 1);
        if (!cont)
            state = EhloDone;
    }
    if (state != EhloDone)
        return;
    if (extensions.contains("STARTTLS") && !disableStartTLS) {
        startTLS();
    } else {
        authenticate();
    }
}

void QxtSmtpPrivate::startTLS()
{
    socket->write("starttls\r\n");
    state = StartTLSSent;
}

void QxtSmtpPrivate::authenticate()
{
    if (!extensions.contains("AUTH") || username.isEmpty() || password.isEmpty()) {
        state = Authenticated;
        emit qxt_p().authenticated();
    } else {
        QStringList auth = extensions["AUTH"].toUpper().split(' ', Qt::SkipEmptyParts);
        if (auth.contains("CRAM-MD5")) {
            authCramMD5();
        } else if (auth.contains("PLAIN")) {
            authPlain();
        } else if (auth.contains("LOGIN")) {
            authLogin();
        } else {
            state = Authenticated;
            emit qxt_p().authenticated();
        }
    }
}

void QxtSmtpPrivate::authCramMD5(const QByteArray &challenge)
{
    if (state != AuthRequestSent) {
        socket->write("auth cram-md5\r\n");
        authType = AuthCramMD5;
        state = AuthRequestSent;
    } else {
        QxtHmac hmac(QCryptographicHash::Md5);
        hmac.setKey(password);
        hmac.addData(QByteArray::fromBase64(challenge));
        QByteArray response = username + ' ' + hmac.result().toHex();
        socket->write(response.toBase64() + "\r\n");
        state = AuthSent;
    }
}

void QxtSmtpPrivate::authPlain()
{
    if (state != AuthRequestSent) {
        socket->write("auth plain\r\n");
        authType = AuthPlain;
        state = AuthRequestSent;
    } else {
        QByteArray auth;
        auth += '\0';
        auth += username;
        auth += '\0';
        auth += password;
        socket->write(auth.toBase64() + "\r\n");
        state = AuthSent;
    }
}

void QxtSmtpPrivate::authLogin()
{
    if (state != AuthRequestSent && state != AuthUsernameSent) {
        socket->write("auth login\r\n");
        authType = AuthLogin;
        state = AuthRequestSent;
    } else if (state == AuthRequestSent) {
        socket->write(username.toBase64() + "\r\n");
        state = AuthUsernameSent;
    } else {
        socket->write(password.toBase64() + "\r\n");
        state = AuthSent;
    }
}

static QByteArray qxt_extract_address(const QString &address)
{
    int parenDepth = 0;
    int addrStart = -1;
    bool inQuote = false;
    int ct = address.length();

    for (int i = 0; i < ct; i++) {
        QChar ch = address[i];
        if (inQuote) {
            if (ch == '"')
                inQuote = false;
        } else if (addrStart != -1) {
            if (ch == '>')
                return address.mid(addrStart, (i - addrStart)).toLatin1();
        } else if (ch == '(') {
            parenDepth++;
        } else if (ch == ')') {
            parenDepth--;
            if (parenDepth < 0)
                parenDepth = 0;
        } else if (ch == '"') {
            if (parenDepth == 0)
                inQuote = true;
        } else if (ch == '<') {
            if (!inQuote && parenDepth == 0)
                addrStart = i + 1;
        }
    }
    return address.toLatin1();
}

void QxtSmtpPrivate::sendNext()
{
    if (state == Disconnected) {
        // leave the mail in the queue if not ready to send
        return;
    }

    if (pending.isEmpty()) {
        // if there are no additional mails to send, finish up
        state = Waiting;
        emit qxt_p().finished();
        return;
    }

    if (state != Waiting) {
        state = Resetting;
        socket->write("rset\r\n");
        return;
    }
    const QxtMailMessage &msg = pending.first().second;
    rcptNumber = rcptAck = mailAck = 0;
    recipients =
        msg.recipients(QxtMailMessage::To) + msg.recipients(QxtMailMessage::Cc) + msg.recipients(QxtMailMessage::Bcc);
    if (recipients.count() == 0) {
        // can't send an e-mail with no recipients
        emit qxt_p().mailFailed(pending.first().first, QxtSmtp::NoRecipients);
        emit qxt_p().mailFailed(pending.first().first, QxtSmtp::NoRecipients, QByteArray("e-mail has no recipients"));
        pending.removeFirst();
        sendNext();
        return;
    }
    // We explicitly use lowercase keywords because for some reason gmail
    // interprets any string starting with an uppercase R as a request
    // to renegotiate the SSL connection.
    socket->write("mail from:<" + qxt_extract_address(msg.sender()) + ">\r\n");
    if (extensions.contains("PIPELINING")) // almost all do nowadays
    {
        for (const QString &rcpt : recipients) {
            socket->write("rcpt to:<" + qxt_extract_address(rcpt) + ">\r\n");
        }
        state = RcptAckPending;
    } else {
        state = MailToSent;
    }
}

void QxtSmtpPrivate::sendNextRcpt(const QByteArray &code, const QByteArray &line)
{
    int messageID = pending.first().first;
    const QxtMailMessage &msg = pending.first().second;

    if (code[0] != '2') {
        // on failure, emit a warning signal
        if (!mailAck) {
            emit qxt_p().senderRejected(messageID, msg.sender());
            emit qxt_p().senderRejected(messageID, msg.sender(), line);
        } else {
            emit qxt_p().recipientRejected(messageID, msg.sender());
            emit qxt_p().recipientRejected(messageID, msg.sender(), line);
        }
    } else if (!mailAck) {
        mailAck = true;
    } else {
        rcptAck++;
    }

    if (rcptNumber == recipients.count()) {
        // all recipients have been sent
        if (rcptAck == 0) {
            // no recipients were considered valid
            emit qxt_p().mailFailed(messageID, code.toInt());
            emit qxt_p().mailFailed(messageID, code.toInt(), line);
            pending.removeFirst();
            sendNext();
        } else {
            // at least one recipient was acknowledged, send mail body
            socket->write("data\r\n");
            state = SendingBody;
        }
    } else if (state != RcptAckPending) {
        // send the next recipient unless we're only waiting on acks
        socket->write("rcpt to:<" + qxt_extract_address(recipients[rcptNumber]) + ">\r\n");
        rcptNumber++;
    } else {
        // If we're only waiting on acks, just count them
        rcptNumber++;
    }
}

void QxtSmtpPrivate::sendBody(const QByteArray &code, const QByteArray &line)
{
    int messageID = pending.first().first;
    const QxtMailMessage &msg = pending.first().second;

    if (code[0] != '3') {
        emit qxt_p().mailFailed(messageID, code.toInt());
        emit qxt_p().mailFailed(messageID, code.toInt(), line);
        pending.removeFirst();
        sendNext();
        return;
    }

    socket->write(msg.rfc2822());
    socket->write(".\r\n");
    state = BodySent;
}
