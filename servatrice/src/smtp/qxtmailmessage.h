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
#ifndef QXTMAILMESSAGE_H
#define QXTMAILMESSAGE_H

#include "qxtglobal.h"
#include "qxtmailattachment.h"

#include <QStringList>
#include <QHash>
#include <QMetaType>
#include <QSharedDataPointer>

struct QxtMailMessagePrivate;
class QXT_NETWORK_EXPORT QxtMailMessage
{
public:
    enum RecipientType
    {
        To,
        Cc,
        Bcc
    };

    QxtMailMessage();
    QxtMailMessage(const QxtMailMessage& other);
    QxtMailMessage(const QString& sender, const QString& recipient);
    QxtMailMessage& operator=(const QxtMailMessage& other);
    ~QxtMailMessage();

    QString sender() const;
    void setSender(const QString&);

    QString subject() const;
    void setSubject(const QString&);

    QString body() const;
    void setBody(const QString&);

    QStringList recipients(RecipientType type = To) const;
    void addRecipient(const QString&, RecipientType type = To);
    void removeRecipient(const QString&);

    QHash<QString, QString> extraHeaders() const;
    QString extraHeader(const QString&) const;
    bool hasExtraHeader(const QString&) const;
    void setExtraHeader(const QString& key, const QString& value);
    void setExtraHeaders(const QHash<QString, QString>&);
    void removeExtraHeader(const QString& key);

    QHash<QString, QxtMailAttachment> attachments() const;
    QxtMailAttachment attachment(const QString& filename) const;
    void addAttachment(const QString& filename, const QxtMailAttachment& attach);
    void removeAttachment(const QString& filename);

    QByteArray rfc2822() const;

private:
    QSharedDataPointer<QxtMailMessagePrivate> qxt_d;
};
Q_DECLARE_TYPEINFO(QxtMailMessage, Q_MOVABLE_TYPE);

#endif // QXTMAIL_H
