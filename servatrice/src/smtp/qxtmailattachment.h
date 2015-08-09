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
#ifndef QXTMAILATTACHMENT_H
#define QXTMAILATTACHMENT_H

#include "qxtglobal.h"

#include <QStringList>
#include <QHash>
#include <QByteArray>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QIODevice>

struct QxtMailAttachmentPrivate;
class QXT_NETWORK_EXPORT QxtMailAttachment
{
public:
    QxtMailAttachment();
    QxtMailAttachment(const QxtMailAttachment& other);
    QxtMailAttachment(const QByteArray& content, const QString& contentType = QString("application/octet-stream"));
    QxtMailAttachment(QIODevice* content, const QString& contentType = QString("application/octet-stream"));
    QxtMailAttachment& operator=(const QxtMailAttachment& other);
    ~QxtMailAttachment();
    static QxtMailAttachment fromFile(const QString& filename);

    QIODevice* content() const;
    void setContent(const QByteArray& content);
    void setContent(QIODevice* content);

    bool deleteContent() const;
    void setDeleteContent(bool enable);

    QString contentType() const;
    void setContentType(const QString& contentType);

    QHash<QString, QString> extraHeaders() const;
    QString extraHeader(const QString&) const;
    bool hasExtraHeader(const QString&) const;
    void setExtraHeader(const QString& key, const QString& value);
    void setExtraHeaders(const QHash<QString, QString>&);
    void removeExtraHeader(const QString& key);

    QByteArray mimeData();

private:
    QSharedDataPointer<QxtMailAttachmentPrivate> qxt_d;
};
Q_DECLARE_TYPEINFO(QxtMailAttachment, Q_MOVABLE_TYPE);

#endif // QXTMAILATTACHMENT_H
