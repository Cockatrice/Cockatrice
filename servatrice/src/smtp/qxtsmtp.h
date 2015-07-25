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
#ifndef QXTSMTP_H
#define QXTSMTP_H

#include <QObject>
#include <QHostAddress>
#include <QString>

#include "qxtglobal.h"
#include "qxtmailmessage.h"

class QTcpSocket;
#ifndef QT_NO_OPENSSL
class QSslSocket;
#endif

class QxtSmtpPrivate;
class QXT_NETWORK_EXPORT QxtSmtp : public QObject
{
    Q_OBJECT
public:
    enum SmtpError
    {
        NoError,
        NoRecipients,
        CommandUnrecognized = 500,
        SyntaxError,
        CommandNotImplemented,
        BadSequence,
        ParameterNotImplemented,
        MailboxUnavailable = 550,
        UserNotLocal,
        MessageTooLarge,
        InvalidMailboxName,
        TransactionFailed
    };

    QxtSmtp(QObject* parent = 0);

    QByteArray username() const;
    void setUsername(const QByteArray& name);

    QByteArray password() const;
    void setPassword(const QByteArray& password);

    int send(const QxtMailMessage& message);
    int pendingMessages() const;

    QTcpSocket* socket() const;
    void connectToHost(const QString& hostName, quint16 port = 25);
    void connectToHost(const QHostAddress& address, quint16 port = 25);
    void disconnectFromHost();

    bool startTlsDisabled() const;
    void setStartTlsDisabled(bool disable);

#ifndef QT_NO_OPENSSL
    QSslSocket* sslSocket() const;
    void connectToSecureHost(const QString& hostName, quint16 port = 465);
    void connectToSecureHost(const QHostAddress& address, quint16 port = 465);
#endif

    bool hasExtension(const QString& extension);
    QString extensionData(const QString& extension);

Q_SIGNALS:
    void connected();
    void connectionFailed();
    void connectionFailed( const QByteArray & msg );
    void encrypted();
    void encryptionFailed();
    void encryptionFailed( const QByteArray & msg );
    void authenticated();
    void authenticationFailed();
    void authenticationFailed( const QByteArray & msg );

    void senderRejected(int mailID, const QString& address );
    void senderRejected(int mailID, const QString& address, const QByteArray & msg );
    void recipientRejected(int mailID, const QString& address );
    void recipientRejected(int mailID, const QString& address, const QByteArray & msg );
    void mailFailed(int mailID, int errorCode);
    void mailFailed(int mailID, int errorCode, const QByteArray & msg);
    void mailSent(int mailID);

    void finished();
    void disconnected();

private:
    QXT_DECLARE_PRIVATE(QxtSmtp)
};

#endif // QXTSMTP_H
