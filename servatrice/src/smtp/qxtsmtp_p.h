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
#ifndef QXTSMTP_P_H
#define QXTSMTP_P_H

#include "qxtsmtp.h"
#include <QHash>
#include <QString>
#include <QList>
#include <QPair>

class QxtSmtpPrivate : public QObject, public QxtPrivate<QxtSmtp>
{
    Q_OBJECT
public:
    QxtSmtpPrivate();

    QXT_DECLARE_PUBLIC(QxtSmtp)

    enum SmtpState
    {
        Disconnected,
        StartState,
        EhloSent,
        EhloGreetReceived,
        EhloExtensionsReceived,
        EhloDone,
        HeloSent,
        StartTLSSent,
        AuthRequestSent,
        AuthUsernameSent,
        AuthSent,
        Authenticated,
        MailToSent,
        RcptAckPending,
        SendingBody,
        BodySent,
        Waiting,
        Resetting
    };

    enum AuthType
    {
        AuthPlain,
        AuthLogin,
        AuthCramMD5
    };

    bool useSecure, disableStartTLS;
    SmtpState state;// rather then an int use the enum.  makes sure invalid states are entered at compile time, and makes debugging easier
    AuthType authType;
    QByteArray buffer, username, password;
    QHash<QString, QString> extensions;
    QList<QPair<int, QxtMailMessage> > pending;
    QStringList recipients;
    int nextID, rcptNumber, rcptAck;
    bool mailAck;

#ifndef QT_NO_OPENSSL
    QSslSocket* socket;
#else
    QTcpSocket* socket;
#endif

    void parseEhlo(const QByteArray& code, bool cont, const QString& line);
    void startTLS();
    void authenticate();

    void authCramMD5(const QByteArray& challenge = QByteArray());
    void authPlain();
    void authLogin();

    void sendNextRcpt(const QByteArray& code, const QByteArray & line);
    void sendBody(const QByteArray& code, const QByteArray & line);

public slots:
    void socketError(QAbstractSocket::SocketError err);
    void socketRead();

    void ehlo();
    void sendNext();
};

#endif // QXTSMTP_P_H
