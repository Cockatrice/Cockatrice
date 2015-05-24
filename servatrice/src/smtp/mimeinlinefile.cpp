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

#include "mimeinlinefile.h"

/* [1] Constructors and Destructors */

MimeInlineFile::MimeInlineFile(QFile *f)
    : MimeFile(f)
{
}

MimeInlineFile::~MimeInlineFile()
{}

/* [1] --- */


/* [2] Getters and Setters */

/* [2] --- */


/* [3] Protected methods */

void MimeInlineFile::prepare()
{       
    this->header += "Content-Disposition: inline\r\n";

    /* !!! IMPORTANT !!! */
    MimeFile::prepare();
}

/* [3] --- */



