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

#ifndef MIMEMULTIPART_H
#define MIMEMULTIPART_H

#include "mimepart.h"

#include "smtpexports.h"

class SMTP_EXPORT MimeMultiPart : public MimePart
{
    Q_OBJECT
public:

    /* [0] Enums */
    enum MultiPartType {
        Mixed           = 0,            // RFC 2046, section 5.1.3
        Digest          = 1,            // RFC 2046, section 5.1.5
        Alternative     = 2,            // RFC 2046, section 5.1.4
        Related         = 3,            // RFC 2387
        Report          = 4,            // RFC 6522
        Signed          = 5,            // RFC 1847, section 2.1
        Encrypted       = 6             // RFC 1847, section 2.2
    };

    /* [0] --- */

    /* [1] Constructors and Destructors */
    MimeMultiPart(const MultiPartType type = Related);

    ~MimeMultiPart();

    /* [1] --- */

    /* [2] Getters and Setters */

    void setMimeType(const MultiPartType type);
    MultiPartType getMimeType() const;

    const QList<MimePart*> & getParts() const;

    /* [2] --- */

    /* [3] Public methods */

    void addPart(MimePart *part);

    virtual void prepare();

    /* [3] --- */

protected:
    QList< MimePart* > parts;

    MultiPartType type;
    
};

#endif // MIMEMULTIPART_H
