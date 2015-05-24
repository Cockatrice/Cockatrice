/*
  Copyright (c) 2011-2012 - Tőkés Attila

  This file is part of SmtpClient for Qt.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.
*/

#ifndef MIMECONTENTFORMATTER_H
#define MIMECONTENTFORMATTER_H

#include <QObject>
#include <QByteArray>

#include "smtpexports.h"

class SMTP_EXPORT MimeContentFormatter : public QObject
{
    Q_OBJECT
public:
    MimeContentFormatter (int max_length = 76);

    void setMaxLength(int l);
    int getMaxLength() const;

    QString format(const QString &content, bool quotedPrintable = false) const;

protected:
    int max_length;

};

#endif // MIMECONTENTFORMATTER_H
