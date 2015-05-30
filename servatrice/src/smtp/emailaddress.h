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

#ifndef EMAILADDRESS_H
#define EMAILADDRESS_H

#include <QObject>

#include "smtpexports.h"

class SMTP_EXPORT EmailAddress : public QObject
{
    Q_OBJECT
public:

    /* [1] Constructors and Destructors */

    EmailAddress();
    EmailAddress(const QString & address, const QString & name="");

    ~EmailAddress();

    /* [1] --- */


    /* [2] Getters and Setters */
    void setName(const QString & name);
    void setAddress(const QString & address);

    const QString & getName() const;
    const QString & getAddress() const;

    /* [2] --- */


private:

    /* [3] Private members */

    QString name;
    QString address;

    /* [3] --- */
};

#endif // EMAILADDRESS_H
