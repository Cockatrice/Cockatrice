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
 * \class QxtMailAttachment
 * \inmodule QxtNetwork
 * \brief The QxtMailAttachment class represents an attachement to a QxtMailMessage
 */




#include "qxtmailattachment.h"
#include "qxtmail_p.h"
#include <QTextCodec>
#include <QBuffer>
#include <QPointer>
#include <QFile>
#include <QtDebug>

struct QxtMailAttachmentPrivate : public QSharedData
{
    QHash<QString, QString> extraHeaders;
    QString contentType;
    QPointer<QIODevice> content;
    bool deleteContent;

    QxtMailAttachmentPrivate()
    {
        content = 0;
        deleteContent = false;
        contentType = "text/plain";
    }

    ~QxtMailAttachmentPrivate()
    {
        if (deleteContent && content)
            content->deleteLater();
        deleteContent = false;
        content = 0;
    }
};

QxtMailAttachment::QxtMailAttachment()
{
    qxt_d = new QxtMailAttachmentPrivate;
}

QxtMailAttachment::QxtMailAttachment(const QxtMailAttachment& other) : qxt_d(other.qxt_d)
{
    // trivial copy constructor
}

QxtMailAttachment::QxtMailAttachment(const QByteArray& content, const QString& contentType)
{
    qxt_d = new QxtMailAttachmentPrivate;
    setContentType(contentType);
    setContent(content);
}

QxtMailAttachment::QxtMailAttachment(QIODevice* content, const QString& contentType)
{
    qxt_d = new QxtMailAttachmentPrivate;
    setContentType(contentType);
    setContent(content);
}

QxtMailAttachment& QxtMailAttachment::operator=(const QxtMailAttachment & other)
{
    qxt_d = other.qxt_d;
    return *this;
}

QxtMailAttachment::~QxtMailAttachment()
{
    // trivial destructor
}

QIODevice* QxtMailAttachment::content() const
{
    return qxt_d->content;
}

void QxtMailAttachment::setContent(const QByteArray& content)
{
    if (qxt_d->deleteContent && qxt_d->content)
        qxt_d->content->deleteLater();
    qxt_d->content = new QBuffer;
    static_cast<QBuffer*>(qxt_d->content.data())->setData(content);
}

void QxtMailAttachment::setContent(QIODevice* content)
{
    if (qxt_d->deleteContent && qxt_d->content)
        qxt_d->content->deleteLater();
    qxt_d->content = content;
}

bool QxtMailAttachment::deleteContent() const
{
    return qxt_d->deleteContent;
}

void QxtMailAttachment::setDeleteContent(bool enable)
{
    qxt_d->deleteContent = enable;
}

QString QxtMailAttachment::contentType() const
{
    return qxt_d->contentType;
}

void QxtMailAttachment::setContentType(const QString& contentType)
{
    qxt_d->contentType = contentType;
}

QHash<QString, QString> QxtMailAttachment::extraHeaders() const
{
    return qxt_d->extraHeaders;
}

QString QxtMailAttachment::extraHeader(const QString& key) const
{
    return qxt_d->extraHeaders[key.toLower()];
}

bool QxtMailAttachment::hasExtraHeader(const QString& key) const
{
    return qxt_d->extraHeaders.contains(key.toLower());
}

void QxtMailAttachment::setExtraHeader(const QString& key, const QString& value)
{
    qxt_d->extraHeaders[key.toLower()] = value;
}

void QxtMailAttachment::setExtraHeaders(const QHash<QString, QString>& a)
{
    QHash<QString, QString>& headers = qxt_d->extraHeaders;
    headers.clear();
    foreach(const QString& key, a.keys())
    {
        headers[key.toLower()] = a[key];
    }
}

void QxtMailAttachment::removeExtraHeader(const QString& key)
{
    qxt_d->extraHeaders.remove(key.toLower());
}

QByteArray QxtMailAttachment::mimeData()
{
    QIODevice* c = content();
    if (!c)
    {
        qWarning() << "QxtMailAttachment::mimeData(): Content not set or already output";
        return QByteArray();
    }
    if (!c->isOpen() && !c->open(QIODevice::ReadOnly))
    {
        qWarning() << "QxtMailAttachment::mimeData(): Cannot open content for reading";
        return QByteArray();
    }

    QTextCodec* latin1 = QTextCodec::codecForName("latin1");
    QByteArray rv = "Content-Type: " + qxt_d->contentType.toLatin1() + "\r\nContent-Transfer-Encoding: base64\r\n";
    foreach(const QString& r, qxt_d->extraHeaders.keys())
    {
        rv += qxt_fold_mime_header(r.toLatin1(), extraHeader(r), latin1);
    }
    rv += "\r\n";

    while (!c->atEnd())
    {
        rv += c->read(57).toBase64() + "\r\n";
    }
    setContent((QIODevice*)0);
    return rv;
}

QxtMailAttachment QxtMailAttachment::fromFile(const QString& filename)
{
    QxtMailAttachment rv(new QFile(filename));
    rv.setDeleteContent(true);
    return rv;
}
