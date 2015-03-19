/****************************************************************************
** Filename: ZipEntryP.h
** Last updated [dd/mm/yyyy]: 27/03/2011
**
** Wrapper for a ZIP local header.
**
** Some of the code has been inspired by other open source projects,
** (mainly Info-Zip and Gilles Vollant's minizip).
** Compression and decompression actually uses the zlib library.
**
** Copyright (C) 2007-2012 Angius Fabrizio. All rights reserved.
**
** This file is part of the OSDaB project (http://osdab.42cows.org/).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See the file LICENSE.GPL that came with this software distribution or
** visit http://www.gnu.org/copyleft/gpl.html for GPL licensing information.
**
**********************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Zip/UnZip API.  It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef OSDAB_ZIPENTRY_P__H
#define OSDAB_ZIPENTRY_P__H

#include <QtCore/QString>
#include <QtCore/QtGlobal>

OSDAB_BEGIN_NAMESPACE(Zip)

class ZipEntryP
{
public:
    ZipEntryP() :
        lhOffset(0),
        dataOffset(0),
        gpFlag(),
        compMethod(0),
        modTime(),
        modDate(),
        crc(0),
        szComp(0),
        szUncomp(0),
        absolutePath(),
        fileSize(0),
        lhEntryChecked(false)
    {
        gpFlag[0] = gpFlag[1] = 0;
        modTime[0] = modTime[1] = 0;
        modDate[0] = modDate[1] = 0;
	}

	quint32 lhOffset;			// Offset of the local header record for this entry
	mutable quint32 dataOffset;	// Offset of the file data for this entry
	unsigned char gpFlag[2];	// General purpose flag
	quint16 compMethod;			// Compression method
	unsigned char modTime[2];	// Last modified time
	unsigned char modDate[2];	// Last modified date
	quint32 crc;				// CRC32
	quint32 szComp;				// Compressed file size
	quint32 szUncomp;			// Uncompressed file size
    QString comment;			// File comment

    QString absolutePath;       // Internal use
    qint64 fileSize;            // Internal use

    mutable bool lhEntryChecked;		// Is true if the local header record for this entry has been parsed

	inline bool isEncrypted() const { return gpFlag[0] & 0x01; }
	inline bool hasDataDescriptor() const { return gpFlag[0] & 0x08; }
};

OSDAB_END_NAMESPACE

#endif // OSDAB_ZIPENTRY_P__H
