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

#include "emailaddress.h"

/* [1] Constructors and Destructors */

EmailAddress::EmailAddress(const QString & address, const QString & name)
{
    this->address = address;
    this->name = name;
}

EmailAddress::~EmailAddress()
{
}

/* [1] --- */


/* [2] Getters and Setters */

void EmailAddress::setName(const QString & name)
{
    this->name = name;

}

void EmailAddress::setAddress(const QString & address)
{
    this->address = address;
}

const QString & EmailAddress::getName() const
{
    return name;
}

const QString & EmailAddress::getAddress() const
{
    return address;
}

/* [2] --- */

