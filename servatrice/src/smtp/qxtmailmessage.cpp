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
 * \class QxtMailMessage
 * \inmodule QxtNetwork
 * \brief The QxtMailMessage class encapsulates an e-mail according to RFC 2822 and related specifications
 */
//! \todo {implicitshared}
#include "qxtmailmessage.h"

#include "qxtmail_p.h"

#include <QDir>
#include <QUuid>
#include <QtDebug>

static bool isASCII(const QString &string) {
  for(const QChar &chr : string){
    if(chr.unicode() > 0x7f)
      return false;
  }
  return true;
}

struct QxtMailMessagePrivate : public QSharedData
{
    QxtMailMessagePrivate()
    {
    }
    QxtMailMessagePrivate(const QxtMailMessagePrivate &other)
        : QSharedData(other), rcptTo(other.rcptTo), rcptCc(other.rcptCc), rcptBcc(other.rcptBcc),
          subject(other.subject), body(other.body), sender(other.sender), extraHeaders(other.extraHeaders),
          attachments(other.attachments)
    {
    }
    QStringList rcptTo, rcptCc, rcptBcc;
    QString subject, body, sender;
    QHash<QString, QString> extraHeaders;
    QHash<QString, QxtMailAttachment> attachments;
    mutable QByteArray boundary;
};

QxtMailMessage::QxtMailMessage()
{
    qxt_d = new QxtMailMessagePrivate;
}

QxtMailMessage::QxtMailMessage(const QxtMailMessage &other) : qxt_d(other.qxt_d)
{
    // trivial copy constructor
}

QxtMailMessage::QxtMailMessage(const QString &sender, const QString &recipient)
{
    qxt_d = new QxtMailMessagePrivate;
    setSender(sender);
    addRecipient(recipient);
}

QxtMailMessage::~QxtMailMessage()
{
    // trivial destructor
}

QxtMailMessage &QxtMailMessage::operator=(const QxtMailMessage &other)
{
    qxt_d = other.qxt_d;
    return *this;
}

QString QxtMailMessage::sender() const
{
    return qxt_d->sender;
}

void QxtMailMessage::setSender(const QString &a)
{
    qxt_d->sender = a;
}

QString QxtMailMessage::subject() const
{
    return qxt_d->subject;
}

void QxtMailMessage::setSubject(const QString &a)
{
    qxt_d->subject = a;
}

QString QxtMailMessage::body() const
{
    return qxt_d->body;
}

void QxtMailMessage::setBody(const QString &a)
{
    qxt_d->body = a;
}

QStringList QxtMailMessage::recipients(QxtMailMessage::RecipientType type) const
{
    if (type == Bcc)
        return qxt_d->rcptBcc;
    if (type == Cc)
        return qxt_d->rcptCc;
    return qxt_d->rcptTo;
}

void QxtMailMessage::addRecipient(const QString &a, QxtMailMessage::RecipientType type)
{
    if (type == Bcc)
        qxt_d->rcptBcc.append(a);
    else if (type == Cc)
        qxt_d->rcptCc.append(a);
    else
        qxt_d->rcptTo.append(a);
}

void QxtMailMessage::removeRecipient(const QString &a)
{
    qxt_d->rcptTo.removeAll(a);
    qxt_d->rcptCc.removeAll(a);
    qxt_d->rcptBcc.removeAll(a);
}

QHash<QString, QString> QxtMailMessage::extraHeaders() const
{
    return qxt_d->extraHeaders;
}

QByteArray QxtMailMessage::extraHeader(const QString &key) const
{
    return qxt_d->extraHeaders[key.toLower()].toLatin1();
}

bool QxtMailMessage::hasExtraHeader(const QString &key) const
{
    return qxt_d->extraHeaders.contains(key.toLower());
}

void QxtMailMessage::setExtraHeader(const QString &key, const QString &value)
{
    qxt_d->extraHeaders[key.toLower()] = value;
}

void QxtMailMessage::setExtraHeaders(const QHash<QString, QString> &a)
{
    QHash<QString, QString> &headers = qxt_d->extraHeaders;
    headers.clear();
    for (const QString &key : a.keys()) {
        headers[key.toLower()] = a[key];
    }
}

void QxtMailMessage::removeExtraHeader(const QString &key)
{
    qxt_d->extraHeaders.remove(key.toLower());
}

QHash<QString, QxtMailAttachment> QxtMailMessage::attachments() const
{
    return qxt_d->attachments;
}

QxtMailAttachment QxtMailMessage::attachment(const QString &filename) const
{
    return qxt_d->attachments[filename];
}

void QxtMailMessage::addAttachment(const QString &filename, const QxtMailAttachment &attach)
{
    if (qxt_d->attachments.contains(filename)) {
        qWarning() << "QxtMailMessage::addAttachment: " << filename << " already in use";
        int i = 1;
        while (qxt_d->attachments.contains(filename + "." + QString::number(i))) {
            i++;
        }
        qxt_d->attachments[filename + "." + QString::number(i)] = attach;
    } else {
        qxt_d->attachments[filename] = attach;
    }
}

void QxtMailMessage::removeAttachment(const QString &filename)
{
    qxt_d->attachments.remove(filename);
}

QByteArray qxt_fold_mime_header(const QString &key, const QString &value, const QByteArray &prefix)
{
    QByteArray rv = "";
    QByteArray line = key.toLatin1() + ": ";
    if (!prefix.isEmpty())
        line += prefix;
    if (!value.contains("=?") && isASCII(value)) {
        bool firstWord = true;
        for (const QByteArray &word : value.toLatin1().split(' ')) {
            if (line.size() > 78) {
                rv = rv + line + "\r\n";
                line.clear();
            }
            if (firstWord)
                line += word;
            else
                line += " " + word;
            firstWord = false;
        }
    } else {
        // The text cannot be losslessly encoded as Latin-1. Therefore, we
        // must use quoted-printable or base64 encoding. This is a quick
        // heuristic based on the first 100 characters to see which
        // encoding to use.
        QByteArray utf8 = value.toUtf8();
        int ct = utf8.length();
        int nonAscii = 0;
        for (int i = 0; i < ct && i < 100; i++) {
            if (QXT_MUST_QP(utf8[i]))
                nonAscii++;
        }
        if (nonAscii > 20) {
            // more than 20%-ish non-ASCII characters: use base64
            QByteArray base64 = utf8.toBase64();
            ct = base64.length();
            line += "=?utf-8?b?";
            for (int i = 0; i < ct; i += 4) {
                if (line.length() > 72) {
                    rv += line + "?\r\n";
                    line = " =?utf-8?b?";
                }
                line = line + base64.mid(i, 4);
            }
        } else {
            // otherwise use Q-encoding
            line += "=?utf-8?q?";
            for (int i = 0; i < ct; i++) {
                if (line.length() > 73) {
                    rv += line + "?\r\n";
                    line = " =?utf-8?q?";
                }
                if (QXT_MUST_QP(utf8[i]) || utf8[i] == ' ') {
                    line += "=" + utf8.mid(i, 1).toHex().toUpper();
                } else {
                    line += utf8[i];
                }
            }
        }
        line += "?="; // end encoded-word atom
    }
    return rv + line + "\r\n";
}

QByteArray QxtMailMessage::rfc2822() const
{
    // Use quoted-printable if requested
    bool useQuotedPrintable = (extraHeader("Content-Transfer-Encoding").toLower() == "quoted-printable");
    // Use base64 if requested
    bool useBase64 = (extraHeader("Content-Transfer-Encoding").toLower() == "base64");
    // Check to see if plain text is ASCII-clean; assume it isn't if QP or base64 was requested
    bool bodyIsAscii = !useQuotedPrintable && !useBase64 && isASCII(body());

    QHash<QString, QxtMailAttachment> attach = attachments();
    QByteArray rv;

    if (!sender().isEmpty() && !hasExtraHeader("From")) {
        rv += qxt_fold_mime_header("From", sender());
    }

    if (!qxt_d->rcptTo.isEmpty()) {
        rv += qxt_fold_mime_header("To", qxt_d->rcptTo.join(", "));
    }

    if (!qxt_d->rcptCc.isEmpty()) {
        rv += qxt_fold_mime_header("Cc", qxt_d->rcptCc.join(", "));
    }

    if (!subject().isEmpty()) {
        rv += qxt_fold_mime_header("Subject", subject());
    }

    if (!bodyIsAscii) {
        if (!hasExtraHeader("MIME-Version") && !attach.count())
            rv += "MIME-Version: 1.0\r\n";

        // If no transfer encoding has been requested, guess.
        // Heuristic: If >20% of the first 100 characters aren't
        // 7-bit clean, use base64, otherwise use Q-P.
        if (!bodyIsAscii && !useQuotedPrintable && !useBase64) {
            QString b = body();
            int nonAscii = 0;
            int ct = b.length();
            for (int i = 0; i < ct && i < 100; i++) {
                if (QXT_MUST_QP(b[i]))
                    nonAscii++;
            }
            useQuotedPrintable = !(nonAscii > 20);
            useBase64 = !useQuotedPrintable;
        }
    }

    if (attach.count()) {
        if (qxt_d->boundary.isEmpty())
            qxt_d->boundary = QUuid::createUuid().toString().toLatin1().replace("{", "").replace("}", "");
        if (!hasExtraHeader("MIME-Version"))
            rv += "MIME-Version: 1.0\r\n";
        if (!hasExtraHeader("Content-Type"))
            rv += "Content-Type: multipart/mixed; boundary=" + qxt_d->boundary + "\r\n";
    } else if (!bodyIsAscii && !hasExtraHeader("Content-Transfer-Encoding")) {
        if (!useQuotedPrintable) {
            // base64
            rv += "Content-Transfer-Encoding: base64\r\n";
        } else {
            // quoted-printable
            rv += "Content-Transfer-Encoding: quoted-printable\r\n";
        }
    }

    for (const QString &r : qxt_d->extraHeaders.keys()) {
        if ((r.toLower() == "content-type" || r.toLower() == "content-transfer-encoding") && attach.count()) {
            // Since we're in multipart mode, we'll be outputting this later
            continue;
        }
        rv += qxt_fold_mime_header(r.toLatin1(), extraHeader(r));
    }

    rv += "\r\n";

    if (attach.count()) {
        // we're going to have attachments, so output the lead-in for the message body
        rv += "This is a message with multiple parts in MIME format.\r\n";
        rv += "--" + qxt_d->boundary + "\r\nContent-Type: ";
        if (hasExtraHeader("Content-Type"))
            rv += extraHeader("Content-Type") + "\r\n";
        else
            rv += "text/plain; charset=UTF-8\r\n";
        if (hasExtraHeader("Content-Transfer-Encoding")) {
            rv += "Content-Transfer-Encoding: " + extraHeader("Content-Transfer-Encoding") + "\r\n";
        } else if (!bodyIsAscii) {
            if (!useQuotedPrintable) {
                // base64
                rv += "Content-Transfer-Encoding: base64\r\n";
            } else {
                // quoted-printable
                rv += "Content-Transfer-Encoding: quoted-printable\r\n";
            }
        }
        rv += "\r\n";
    }

    if (bodyIsAscii) {
        QByteArray b = body().toLatin1();
        int len = b.length();
        QByteArray line = "";
        QByteArray word = "";
        for (int i = 0; i < len; i++) {
            if (b[i] == '\n' || b[i] == '\r') {
                if (line.isEmpty()) {
                    line = word;
                    word = "";
                } else if (line.length() + word.length() + 1 <= 78) {
                    line = line + ' ' + word;
                    word = "";
                }
                if (line.isEmpty())
                    continue;
                if (line[0] == '.')
                    rv += ".";
                rv += line + "\r\n";
                if ((b[i + 1] == '\n' || b[i + 1] == '\r') && b[i] != b[i + 1]) {
                    // If we're looking at a CRLF pair, skip the second half
                    i++;
                }
                line = word;
            } else if (b[i] == ' ') {
                if (line.length() + word.length() + 1 > 78) {
                    if (line[0] == '.')
                        rv += ".";
                    rv += line + "\r\n";
                    line = word;
                } else if (line.isEmpty()) {
                    line = word;
                } else {
                    line = line + ' ' + word;
                }
                word = "";
            } else {
                word += b[i];
            }
        }
        if (line.length() + word.length() + 1 > 78) {
            if (line[0] == '.')
                rv += ".";
            rv += line + "\r\n";
            line = word;
        } else if (!word.isEmpty()) {
            line += ' ' + word;
        }
        if (!line.isEmpty()) {
            if (line[0] == '.')
                rv += ".";
            rv += line + "\r\n";
        }
    } else if (useQuotedPrintable) {
        QByteArray b = body().toUtf8();
        int ct = b.length();
        QByteArray line;
        for (int i = 0; i < ct; i++) {
            if (b[i] == '\n' || b[i] == '\r') {
                if (line[0] == '.')
                    rv += ".";
                rv += line + "\r\n";
                line = "";
                if ((b[i + 1] == '\n' || b[i + 1] == '\r') && b[i] != b[i + 1]) {
                    // If we're looking at a CRLF pair, skip the second half
                    i++;
                }
            } else if (line.length() > 74) {
                rv += line + "=\r\n";
                line = "";
            }
            if (QXT_MUST_QP(b[i])) {
                line += "=" + b.mid(i, 1).toHex().toUpper();
            } else {
                line += b[i];
            }
        }
        if (!line.isEmpty()) {
            if (line[0] == '.')
                rv += ".";
            rv += line + "\r\n";
        }
    } else /* base64 */
    {
        QByteArray b = body().toUtf8().toBase64();
        int ct = b.length();
        for (int i = 0; i < ct; i += 78) {
            rv += b.mid(i, 78) + "\r\n";
        }
    }

    if (attach.count()) {
        for (const QString &filename : attach.keys()) {
            rv += "--" + qxt_d->boundary + "\r\n";
            rv +=
                qxt_fold_mime_header("Content-Disposition", QDir(filename).dirName(), "attachment; filename=");
            rv += attach[filename].mimeData();
        }
        rv += "--" + qxt_d->boundary + "--\r\n";
    }

    return rv;
}
