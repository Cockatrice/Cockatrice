/****************************************************************************
** Filename: unzip.cpp
** Last updated [dd/mm/yyyy]: 08/07/2010
**
** pkzip 2.0 decompression.
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

#include "unzip.h"
#include "unzip_p.h"
#include "zipentry_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QStringList>

// You can remove this #include if you replace the qDebug() statements.
#include <QtCore/QtDebug>

/*!
 \class UnZip unzip.h

 \brief PKZip 2.0 file decompression.
 Compatibility with later versions is not ensured as they may use
 unsupported compression algorithms.
 Versions after 2.7 may have an incompatible header format and thus be
 completely incompatible.
*/

/*! \enum UnZip::ErrorCode The result of a decompression operation.
 \value UnZip::Ok No error occurred.
 \value UnZip::ZlibInit Failed to init or load the zlib library.
 \value UnZip::ZlibError The zlib library returned some error.
 \value UnZip::OpenFailed Unable to create or open a device.
 \value UnZip::PartiallyCorrupted Corrupted zip archive - some files could be extracted.
 \value UnZip::Corrupted Corrupted or invalid zip archive.
 \value UnZip::WrongPassword Unable to decrypt a password protected file.
 \value UnZip::NoOpenArchive No archive has been opened yet.
 \value UnZip::FileNotFound Unable to find the requested file in the archive.
 \value UnZip::ReadFailed Reading of a file failed.
 \value UnZip::WriteFailed Writing of a file failed.
 \value UnZip::SeekFailed Seek failed.
 \value UnZip::CreateDirFailed Could not create a directory.
 \value UnZip::InvalidDevice A null device has been passed as parameter.
 \value UnZip::InvalidArchive This is not a valid (or supported) ZIP archive.
 \value UnZip::HeaderConsistencyError Local header record info does not match with the central directory record info. The archive may be corrupted.

 \value UnZip::Skip Internal use only.
 \value UnZip::SkipAll Internal use only.
*/

/*! \enum UnZip::ExtractionOptions Some options for the file extraction methods.
 \value UnZip::ExtractPaths Default. Does not ignore the path of the zipped files.
 \value UnZip::SkipPaths Default. Ignores the path of the zipped files and extracts them all to the same root directory.
 \value UnZip::VerifyOnly Doesn't actually extract files.
 \value UnZip::NoSilentDirectoryCreation Doesn't attempt to silently create missing output directories.
*/

//! Local header size (excluding signature, excluding variable length fields)
#define UNZIP_LOCAL_HEADER_SIZE 26
//! Central Directory file entry size (excluding signature, excluding variable length fields)
#define UNZIP_CD_ENTRY_SIZE_NS 42
//! Data descriptor size (excluding signature)
#define UNZIP_DD_SIZE 12
//! End Of Central Directory size (including signature, excluding variable length fields)
#define UNZIP_EOCD_SIZE 22
//! Local header entry encryption header size
#define UNZIP_LOCAL_ENC_HEADER_SIZE 12

// Some offsets inside a CD record (excluding signature)
#define UNZIP_CD_OFF_VERSION_MADE 0
#define UNZIP_CD_OFF_VERSION 2
#define UNZIP_CD_OFF_GPFLAG 4
#define UNZIP_CD_OFF_CMETHOD 6
#define UNZIP_CD_OFF_MODT 8
#define UNZIP_CD_OFF_MODD 10
#define UNZIP_CD_OFF_CRC32 12
#define UNZIP_CD_OFF_CSIZE 16
#define UNZIP_CD_OFF_USIZE 20
#define UNZIP_CD_OFF_NAMELEN 24
#define UNZIP_CD_OFF_XLEN 26
#define UNZIP_CD_OFF_COMMLEN 28
#define UNZIP_CD_OFF_LHOFFSET 38

// Some offsets inside a local header record (excluding signature)
#define UNZIP_LH_OFF_VERSION 0
#define UNZIP_LH_OFF_GPFLAG 2
#define UNZIP_LH_OFF_CMETHOD 4
#define UNZIP_LH_OFF_MODT 6
#define UNZIP_LH_OFF_MODD 8
#define UNZIP_LH_OFF_CRC32 10
#define UNZIP_LH_OFF_CSIZE 14
#define UNZIP_LH_OFF_USIZE 18
#define UNZIP_LH_OFF_NAMELEN 22
#define UNZIP_LH_OFF_XLEN 24

// Some offsets inside a data descriptor record (excluding signature)
#define UNZIP_DD_OFF_CRC32 0
#define UNZIP_DD_OFF_CSIZE 4
#define UNZIP_DD_OFF_USIZE 8

// Some offsets inside a EOCD record
#define UNZIP_EOCD_OFF_ENTRIES 6
#define UNZIP_EOCD_OFF_CDOFF 12
#define UNZIP_EOCD_OFF_COMMLEN 16

/*!
 Max version handled by this API.
 0x14 = 2.0 --> full compatibility only up to this version;
 later versions use unsupported features
*/
#define UNZIP_VERSION 0x14

//! CRC32 routine
#define CRC32(c, b) crcTable[((int)c^b) & 0xff] ^ (c >> 8)

OSDAB_BEGIN_NAMESPACE(Zip)


/************************************************************************
 ZipEntry
*************************************************************************/

/*!
 ZipEntry constructor - initialize data. Type is set to File.
*/
UnZip::ZipEntry::ZipEntry()
{
    compressedSize = uncompressedSize = crc32 = 0;
    compression = NoCompression;
    type = File;
    encrypted = false;
}


/************************************************************************
 Private interface
*************************************************************************/

//! \internal
UnzipPrivate::UnzipPrivate() :
    password(),
    skipAllEncrypted(false),
    headers(0),
    device(0),
    file(0),
    uBuffer(0),
    crcTable(0),
    cdOffset(0),
    eocdOffset(0),
    cdEntryCount(0),
    unsupportedEntryCount(0),
    comment()
{
    uBuffer = (unsigned char*) buffer1;
    crcTable = (quint32*) get_crc_table();
}

//! \internal
void UnzipPrivate::deviceDestroyed(QObject*)
{
    qDebug("Unexpected device destruction detected.");
    do_closeArchive();
}

//! \internal Parses a Zip archive.
UnZip::ErrorCode UnzipPrivate::openArchive(QIODevice* dev)
{
    Q_ASSERT(!device);
    Q_ASSERT(dev);

    if (!(dev->isOpen() || dev->open(QIODevice::ReadOnly))) {
        qDebug() << "Unable to open device for reading";
        return UnZip::OpenFailed;
    }

    device = dev;
    if (device != file)
        connect(device, SIGNAL(destroyed(QObject*)), this, SLOT(deviceDestroyed(QObject*)));

    UnZip::ErrorCode ec;

    ec = seekToCentralDirectory();
    if (ec != UnZip::Ok) {
        closeArchive();
        return ec;
    }

    //! \todo Ignore CD entry count? CD may be corrupted.
    if (cdEntryCount == 0) {
        return UnZip::Ok;
    }

    bool continueParsing = true;

    while (continueParsing) {
        if (device->read(buffer1, 4) != 4) {
            if (headers) {
                qDebug() << "Corrupted zip archive. Some files might be extracted.";
                ec = headers->size() != 0 ? UnZip::PartiallyCorrupted : UnZip::Corrupted;
                break;
            } else {
                closeArchive();
                qDebug() << "Corrupted or invalid zip archive. Closing.";
                ec = UnZip::Corrupted;
                break;
            }
        }

        if (! (buffer1[0] == 'P' && buffer1[1] == 'K' && buffer1[2] == 0x01  && buffer1[3] == 0x02) )
            break;

        if ((ec = parseCentralDirectoryRecord()) != UnZip::Ok)
            break;
    }

    if (ec != UnZip::Ok)
        closeArchive();

    return ec;
}

/*
 \internal Parses a local header record and makes some consistency check
 with the information stored in the Central Directory record for this entry
 that has been previously parsed.
 \todo Optional consistency check (as a ExtractionOptions flag)

 local file header signature     4 bytes  (0x04034b50)
 version needed to extract       2 bytes
 general purpose bit flag        2 bytes
 compression method              2 bytes
 last mod file time              2 bytes
 last mod file date              2 bytes
 crc-32                          4 bytes
 compressed size                 4 bytes
 uncompressed size               4 bytes
 file name length                2 bytes
 extra field length              2 bytes

 file name (variable size)
 extra field (variable size)
*/
UnZip::ErrorCode UnzipPrivate::parseLocalHeaderRecord(const QString& path, const ZipEntryP& entry)
{
    Q_ASSERT(device);

    if (!device->seek(entry.lhOffset))
        return UnZip::SeekFailed;

    // Test signature
    if (device->read(buffer1, 4) != 4)
        return UnZip::ReadFailed;

    if ((buffer1[0] != 'P') || (buffer1[1] != 'K') || (buffer1[2] != 0x03) || (buffer1[3] != 0x04))
        return UnZip::InvalidArchive;

    if (device->read(buffer1, UNZIP_LOCAL_HEADER_SIZE) != UNZIP_LOCAL_HEADER_SIZE)
        return UnZip::ReadFailed;

    /*
  Check 3rd general purpose bit flag.

  "bit 3: If this bit is set, the fields crc-32, compressed size
  and uncompressed size are set to zero in the local
  header.  The correct values are put in the data descriptor
  immediately following the compressed data."
 */
    bool hasDataDescriptor = entry.hasDataDescriptor();

    bool checkFailed = false;

    if (!checkFailed)
        checkFailed = entry.compMethod != getUShort(uBuffer, UNZIP_LH_OFF_CMETHOD);
    if (!checkFailed)
        checkFailed = entry.gpFlag[0] != uBuffer[UNZIP_LH_OFF_GPFLAG];
    if (!checkFailed)
        checkFailed = entry.gpFlag[1] != uBuffer[UNZIP_LH_OFF_GPFLAG + 1];
    if (!checkFailed)
        checkFailed = entry.modTime[0] != uBuffer[UNZIP_LH_OFF_MODT];
    if (!checkFailed)
        checkFailed = entry.modTime[1] != uBuffer[UNZIP_LH_OFF_MODT + 1];
    if (!checkFailed)
        checkFailed = entry.modDate[0] != uBuffer[UNZIP_LH_OFF_MODD];
    if (!checkFailed)
        checkFailed = entry.modDate[1] != uBuffer[UNZIP_LH_OFF_MODD + 1];
    if (!hasDataDescriptor)
    {
        if (!checkFailed)
            checkFailed = entry.crc != getULong(uBuffer, UNZIP_LH_OFF_CRC32);
        if (!checkFailed)
            checkFailed = entry.szComp != getULong(uBuffer, UNZIP_LH_OFF_CSIZE);
        if (!checkFailed)
            checkFailed = entry.szUncomp != getULong(uBuffer, UNZIP_LH_OFF_USIZE);
    }

    if (checkFailed)
        return UnZip::HeaderConsistencyError;

    // Check filename
    quint16 szName = getUShort(uBuffer, UNZIP_LH_OFF_NAMELEN);
    if (szName == 0)
        return UnZip::HeaderConsistencyError;

    if (device->read(buffer2, szName) != szName)
        return UnZip::ReadFailed;

    QString filename = QString::fromLatin1(buffer2, szName);
    if (filename != path) {
        qDebug() << "Filename in local header mismatches.";
        return UnZip::HeaderConsistencyError;
    }

    // Skip extra field
    quint16 szExtra = getUShort(uBuffer, UNZIP_LH_OFF_XLEN);
    if (szExtra != 0) {
        if (!device->seek(device->pos() + szExtra))
            return UnZip::SeekFailed;
    }

    entry.dataOffset = device->pos();

    if (hasDataDescriptor) {
        /*
   The data descriptor has this OPTIONAL signature: PK\7\8
   We try to skip the compressed data relying on the size set in the
   Central Directory record.
  */
        if (!device->seek(device->pos() + entry.szComp))
            return UnZip::SeekFailed;

        // Read 4 bytes and check if there is a data descriptor signature
        if (device->read(buffer2, 4) != 4)
            return UnZip::ReadFailed;

        bool hasSignature = buffer2[0] == 'P' && buffer2[1] == 'K' && buffer2[2] == 0x07 && buffer2[3] == 0x08;
        if (hasSignature) {
            if (device->read(buffer2, UNZIP_DD_SIZE) != UNZIP_DD_SIZE)
                return UnZip::ReadFailed;
        } else {
            if (device->read(buffer2 + 4, UNZIP_DD_SIZE - 4) != UNZIP_DD_SIZE - 4)
                return UnZip::ReadFailed;
        }

        // DD: crc, compressed size, uncompressed size
        if (
        entry.crc != getULong((unsigned char*)buffer2, UNZIP_DD_OFF_CRC32) ||
        entry.szComp != getULong((unsigned char*)buffer2, UNZIP_DD_OFF_CSIZE) ||
        entry.szUncomp != getULong((unsigned char*)buffer2, UNZIP_DD_OFF_USIZE)
        )
            return UnZip::HeaderConsistencyError;
    }

    return UnZip::Ok;
}

/*! \internal Attempts to find the start of the central directory record.

 We seek the file back until we reach the "End Of Central Directory"
 signature PK\5\6.

 end of central dir signature    4 bytes  (0x06054b50)
 number of this disk             2 bytes
 number of the disk with the
 start of the central directory  2 bytes
 total number of entries in the
 central directory on this disk  2 bytes
 total number of entries in
 the central directory           2 bytes
 size of the central directory   4 bytes
 offset of start of central
 directory with respect to
 the starting disk number        4 bytes
 .ZIP file comment length        2 bytes
 --- SIZE UNTIL HERE: UNZIP_EOCD_SIZE ---
 .ZIP file comment       (variable size)
*/
UnZip::ErrorCode UnzipPrivate::seekToCentralDirectory()
{
    Q_ASSERT(device);

    qint64 length = device->size();
    qint64 offset = length - UNZIP_EOCD_SIZE;

    if (length < UNZIP_EOCD_SIZE)
        return UnZip::InvalidArchive;

    if (!device->seek( offset ))
        return UnZip::SeekFailed;

    if (device->read(buffer1, UNZIP_EOCD_SIZE) != UNZIP_EOCD_SIZE)
        return UnZip::ReadFailed;

    bool eocdFound = (buffer1[0] == 'P' && buffer1[1] == 'K' && buffer1[2] == 0x05 && buffer1[3] == 0x06);

    if (eocdFound) {
        // Zip file has no comment (the only variable length field in the EOCD record)
        eocdOffset = offset;
    } else {
        qint64 read;
        char* p = 0;

        offset -= UNZIP_EOCD_SIZE;

        if (offset <= 0)
            return UnZip::InvalidArchive;

        if (!device->seek( offset ))
            return UnZip::SeekFailed;

        while ((read = device->read(buffer1, UNZIP_EOCD_SIZE)) >= 0) {
            if ( (p = strstr(buffer1, "PK\5\6")) != 0) {
                // Seek to the start of the EOCD record so we can read it fully
                // Yes... we could simply read the missing bytes and append them to the buffer
                // but this is far easier so heck it!
                device->seek( offset + (p - buffer1) );
                eocdFound = true;
                eocdOffset = offset + (p - buffer1);

                // Read EOCD record
                if (device->read(buffer1, UNZIP_EOCD_SIZE) != UNZIP_EOCD_SIZE)
                    return UnZip::ReadFailed;

                break;
            }

            // TODO: This is very slow and only a temporary bug fix. Need some pattern matching algorithm here.
            offset -= 1 /*UNZIP_EOCD_SIZE*/;
            if (offset <= 0)
                return UnZip::InvalidArchive;

            if (!device->seek( offset ))
                return UnZip::SeekFailed;
        }
    }

    if (!eocdFound)
        return UnZip::InvalidArchive;

    // Parse EOCD to locate CD offset
    offset = getULong((const unsigned char*)buffer1, UNZIP_EOCD_OFF_CDOFF + 4);

    cdOffset = offset;

    cdEntryCount = getUShort((const unsigned char*)buffer1, UNZIP_EOCD_OFF_ENTRIES + 4);

    quint16 commentLength = getUShort((const unsigned char*)buffer1, UNZIP_EOCD_OFF_COMMLEN + 4);
    if (commentLength != 0) {
        QByteArray c = device->read(commentLength);
        if (c.count() != commentLength)
            return UnZip::ReadFailed;

        comment = c;
    }

    // Seek to the start of the CD record
    if (!device->seek( cdOffset ))
        return UnZip::SeekFailed;

    return UnZip::Ok;
}

/*!
    \internal Parses a central directory record.

    Central Directory record structure:

    [file header 1]
    .
    .
    .
    [file header n]
    [digital signature] // PKZip 6.2 or later only

    File header:

    central file header signature   4 bytes  (0x02014b50)
    version made by                 2 bytes
    version needed to extract       2 bytes
    general purpose bit flag        2 bytes
    compression method              2 bytes
    last mod file time              2 bytes
    last mod file date              2 bytes
    crc-32                          4 bytes
    compressed size                 4 bytes
    uncompressed size               4 bytes
    file name length                2 bytes
    extra field length              2 bytes
    file comment length             2 bytes
    disk number start               2 bytes
    internal file attributes        2 bytes
    external file attributes        4 bytes
    relative offset of local header 4 bytes

    file name (variable size)
    extra field (variable size)
    file comment (variable size)
*/
UnZip::ErrorCode UnzipPrivate::parseCentralDirectoryRecord()
{
    Q_ASSERT(device);

    // Read CD record
    if (device->read(buffer1, UNZIP_CD_ENTRY_SIZE_NS) != UNZIP_CD_ENTRY_SIZE_NS)
        return UnZip::ReadFailed;

    bool skipEntry = false;

    // Get compression type so we can skip non compatible algorithms
    quint16 compMethod = getUShort(uBuffer, UNZIP_CD_OFF_CMETHOD);

    // Get variable size fields length so we can skip the whole record
    // if necessary
    quint16 szName = getUShort(uBuffer, UNZIP_CD_OFF_NAMELEN);
    quint16 szExtra = getUShort(uBuffer, UNZIP_CD_OFF_XLEN);
    quint16 szComment = getUShort(uBuffer, UNZIP_CD_OFF_COMMLEN);

    quint32 skipLength = szName + szExtra + szComment;

    UnZip::ErrorCode ec = UnZip::Ok;

    if ((compMethod != 0) && (compMethod != 8)) {
        qDebug() << "Unsupported compression method. Skipping file.";
        skipEntry = true;
    }

    if (!skipEntry && szName == 0) {
        qDebug() << "Skipping file with no name.";
        skipEntry = true;
    }

    QString filename;
    if (device->read(buffer2, szName) != szName) {
        ec = UnZip::ReadFailed;
        skipEntry = true;
    } else {
        filename = QString::fromLatin1(buffer2, szName);
    }

    // Unsupported features if version is bigger than UNZIP_VERSION
    if (!skipEntry && buffer1[UNZIP_CD_OFF_VERSION] > UNZIP_VERSION) {
        QString v = QString::number(buffer1[UNZIP_CD_OFF_VERSION]);
        if (v.length() == 2)
            v.insert(1, QLatin1Char('.'));
        v = QString::fromLatin1("Unsupported PKZip version (%1). Skipping file: %2")
            .arg(v, filename.isEmpty() ? QString::fromLatin1("<undefined>") : filename);
        qDebug() << v.toLatin1().constData();
        skipEntry = true;
    }

    if (skipEntry) {
        if (ec == UnZip::Ok) {
            if (!device->seek( device->pos() + skipLength ))
                ec = UnZip::SeekFailed;
            unsupportedEntryCount++;
        }

        return ec;
    }

    ZipEntryP* h = new ZipEntryP;
    h->compMethod = compMethod;

    h->gpFlag[0] = buffer1[UNZIP_CD_OFF_GPFLAG];
    h->gpFlag[1] = buffer1[UNZIP_CD_OFF_GPFLAG + 1];

    h->modTime[0] = buffer1[UNZIP_CD_OFF_MODT];
    h->modTime[1] = buffer1[UNZIP_CD_OFF_MODT + 1];

    h->modDate[0] = buffer1[UNZIP_CD_OFF_MODD];
    h->modDate[1] = buffer1[UNZIP_CD_OFF_MODD + 1];

    h->crc = getULong(uBuffer, UNZIP_CD_OFF_CRC32);
    h->szComp = getULong(uBuffer, UNZIP_CD_OFF_CSIZE);
    h->szUncomp = getULong(uBuffer, UNZIP_CD_OFF_USIZE);

    // Skip extra field (if any)
    if (szExtra != 0) {
        if (!device->seek( device->pos() + szExtra )) {
            delete h;
            return UnZip::SeekFailed;
        }
    }

    // Read comment field (if any)
    if (szComment != 0) {
        if (device->read(buffer2, szComment) != szComment) {
            delete h;
            return UnZip::ReadFailed;
        }

        h->comment = QString::fromLatin1(buffer2, szComment);
    }

    h->lhOffset = getULong(uBuffer, UNZIP_CD_OFF_LHOFFSET);

    if (!headers)
        headers = new QMap<QString, ZipEntryP*>();
    headers->insert(filename, h);

    return UnZip::Ok;
}

//! \internal Closes the archive and resets the internal status.
void UnzipPrivate::closeArchive()
{
    if (!device) {
        Q_ASSERT(!file);
        return;
    }

    if (device != file)
        disconnect(device, 0, this, 0);

    do_closeArchive();
}

//! \internal
void UnzipPrivate::do_closeArchive()
{
    skipAllEncrypted = false;

    if (headers) {
        if (headers)
            qDeleteAll(*headers);
        delete headers;
        headers = 0;
    }

    device = 0;

    if (file)
        delete file;
    file = 0;

    cdOffset = eocdOffset = 0;
    cdEntryCount = 0;
    unsupportedEntryCount = 0;

    comment.clear();
}

//! \internal
UnZip::ErrorCode UnzipPrivate::extractFile(const QString& path, const ZipEntryP& entry,
    const QDir& dir, UnZip::ExtractionOptions options)
{
    QString name(path);
    QString dirname;
    QString directory;

    const bool verify = (options & UnZip::VerifyOnly);
    const int pos = name.lastIndexOf('/');

    // This entry is for a directory
    if (pos == name.length() - 1) {
        if (verify)
            return UnZip::Ok;

        if (options & UnZip::SkipPaths)
            return UnZip::Ok;

        directory = QString("%1/%2").arg(dir.absolutePath()).arg(QDir::cleanPath(name));
        if (!createDirectory(directory)) {
            qDebug() << QString("Unable to create directory: %1").arg(directory);
            return UnZip::CreateDirFailed;
        }

        return UnZip::Ok;
    }

    // Extract path from entry
    if (verify) {
        return extractFile(path, entry, 0, options);
    }

    if (pos > 0) {
        // get directory part
        dirname = name.left(pos);
        if (options & UnZip::SkipPaths) {
            directory = dir.absolutePath();
        } else {
            directory = QString("%1/%2").arg(dir.absolutePath()).arg(QDir::cleanPath(dirname));
            if (!createDirectory(directory)) {
                qDebug() << QString("Unable to create directory: %1").arg(directory);
                return UnZip::CreateDirFailed;
            }
        }
        name = name.right(name.length() - pos - 1);
    } else {
        directory = dir.absolutePath();
    }

    const bool silentDirectoryCreation = !(options & UnZip::NoSilentDirectoryCreation);
    if (silentDirectoryCreation) {
        if (!createDirectory(directory)) {
            qDebug() << QString("Unable to create output directory %1").arg(directory);
            return UnZip::CreateDirFailed;
        }
    }

    name = QString("%1/%2").arg(directory).arg(name);

    QFile outFile(name);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << QString("Unable to open %1 for writing").arg(name);
        return UnZip::OpenFailed;
    }

    UnZip::ErrorCode ec = extractFile(path, entry, &outFile, options);
    outFile.close();

    const QDateTime lastModified = convertDateTime(entry.modDate, entry.modTime);
    const bool setTimeOk = OSDAB_ZIP_MANGLE(setFileTimestamp)(name, lastModified);
    if (!setTimeOk) {
        qDebug() << QString("Unable to set last modified time on file: %1").arg(name);
    }

    if (ec != UnZip::Ok) {
        if (!outFile.remove())
            qDebug() << QString("Unable to remove corrupted file: %1").arg(name);
    }

    return ec;
}

//! \internal
UnZip::ErrorCode UnzipPrivate::extractStoredFile(
    const quint32 szComp, quint32** keys, quint32& myCRC, QIODevice* outDev,
    UnZip::ExtractionOptions options)
{
    const bool verify = (options & UnZip::VerifyOnly);
    const bool isEncrypted = keys != 0;

    uInt rep = szComp / UNZIP_READ_BUFFER;
    uInt rem = szComp % UNZIP_READ_BUFFER;
    uInt cur = 0;

    // extract data
    qint64 read;
    quint64 tot = 0;

    while ( (read = device->read(buffer1, cur < rep ? UNZIP_READ_BUFFER : rem)) > 0 ) {
        if (isEncrypted)
            decryptBytes(*keys, buffer1, read);

        myCRC = crc32(myCRC, uBuffer, read);
        if (!verify) {
            if (outDev->write(buffer1, read) != read)
                return UnZip::WriteFailed;
        }

        cur++;
        tot += read;
        if (tot == szComp)
            break;
    }

    return (read < 0)
        ? UnZip::ReadFailed
        : UnZip::Ok;
}

//! \internal
UnZip::ErrorCode UnzipPrivate::inflateFile(
    const quint32 szComp, quint32** keys, quint32& myCRC, QIODevice* outDev,
    UnZip::ExtractionOptions options)
{
    const bool verify = (options & UnZip::VerifyOnly);
    const bool isEncrypted = keys != 0;
    Q_ASSERT(verify ? true : outDev != 0);

    uInt rep = szComp / UNZIP_READ_BUFFER;
    uInt rem = szComp % UNZIP_READ_BUFFER;
    uInt cur = 0;

    // extract data
    qint64 read;
    quint64 tot = 0;

    /* Allocate inflate state */
    z_stream zstr;
    zstr.zalloc = Z_NULL;
    zstr.zfree = Z_NULL;
    zstr.opaque = Z_NULL;
    zstr.next_in = Z_NULL;
    zstr.avail_in = 0;

    int zret;

    // Use inflateInit2 with negative windowBits to get raw decompression
    if ( (zret = inflateInit2_(&zstr, -MAX_WBITS, ZLIB_VERSION, sizeof(z_stream))) != Z_OK )
        return UnZip::ZlibError;

    int szDecomp;

    // Decompress until deflate stream ends or end of file
    do {
        read = device->read(buffer1, cur < rep ? UNZIP_READ_BUFFER : rem);
        if (!read)
            break;

        if (read < 0) {
            (void)inflateEnd(&zstr);
            return UnZip::ReadFailed;
        }

        if (isEncrypted)
            decryptBytes(*keys, buffer1, read);

        cur++;
        tot += read;

        zstr.avail_in = (uInt) read;
        zstr.next_in = (Bytef*) buffer1;

        // Run inflate() on input until output buffer not full
        do {
            zstr.avail_out = UNZIP_READ_BUFFER;
            zstr.next_out = (Bytef*) buffer2;;

            zret = inflate(&zstr, Z_NO_FLUSH);

            switch (zret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&zstr);
                return UnZip::WriteFailed;
            default:
                ;
            }

            szDecomp = UNZIP_READ_BUFFER - zstr.avail_out;
            if (!verify) {
                if (outDev->write(buffer2, szDecomp) != szDecomp) {
                    inflateEnd(&zstr);
                    return UnZip::ZlibError;
                }
            }

            myCRC = crc32(myCRC, (const Bytef*) buffer2, szDecomp);

        } while (zstr.avail_out == 0);

    } while (zret != Z_STREAM_END);

    inflateEnd(&zstr);
    return UnZip::Ok;
}

//! \internal \p outDev is null if the VerifyOnly option is set
UnZip::ErrorCode UnzipPrivate::extractFile(const QString& path, const ZipEntryP& entry,
    QIODevice* outDev, UnZip::ExtractionOptions options)
{
    const bool verify = (options & UnZip::VerifyOnly);

    Q_UNUSED(options);
    Q_ASSERT(device);
    Q_ASSERT(verify ? true : outDev != 0);

    if (!entry.lhEntryChecked) {
        UnZip::ErrorCode ec = parseLocalHeaderRecord(path, entry);
        entry.lhEntryChecked = true;
        if (ec != UnZip::Ok)
            return ec;
    }

    if (!device->seek(entry.dataOffset))
        return UnZip::SeekFailed;

    // Encryption keys
    quint32 keys[3];
    quint32 szComp = entry.szComp;
    if (entry.isEncrypted()) {
        UnZip::ErrorCode e = testPassword(keys, path, entry);
        if (e != UnZip::Ok)
        {
            qDebug() << QString("Unable to decrypt %1").arg(path);
            return e;
        }//! Encryption header size
        szComp -= UNZIP_LOCAL_ENC_HEADER_SIZE; // remove encryption header size
    }

    if (szComp == 0) {
        if (entry.crc != 0)
            return UnZip::Corrupted;
        return UnZip::Ok;
    }

    quint32 myCRC = crc32(0L, Z_NULL, 0);
    quint32* k = keys;

    UnZip::ErrorCode ec = UnZip::Ok;
    if (entry.compMethod == 0) {
        ec = extractStoredFile(szComp, entry.isEncrypted() ? &k : 0, myCRC, outDev, options);
    } else if (entry.compMethod == 8) {
        ec = inflateFile(szComp, entry.isEncrypted() ? &k : 0, myCRC, outDev, options);
    }

    if (ec == UnZip::Ok && myCRC != entry.crc)
        return UnZip::Corrupted;

    return UnZip::Ok;
}

//! \internal Creates a new directory and all the needed parent directories.
bool UnzipPrivate::createDirectory(const QString& path)
{
    QDir d(path);
    if (!d.exists() && !d.mkpath(path)) {
        qDebug() << QString("Unable to create directory: %1").arg(path);
        return false;
    }

    return true;
}

/*!
 \internal Reads an quint32 (4 bytes) from a byte array starting at given offset.
*/
quint32 UnzipPrivate::getULong(const unsigned char* data, quint32 offset) const
{
    quint32 res = (quint32) data[offset];
    res |= (((quint32)data[offset+1]) << 8);
    res |= (((quint32)data[offset+2]) << 16);
    res |= (((quint32)data[offset+3]) << 24);

    return res;
}

/*!
 \internal Reads an quint64 (8 bytes) from a byte array starting at given offset.
*/
quint64 UnzipPrivate::getULLong(const unsigned char* data, quint32 offset) const
{
    quint64 res = (quint64) data[offset];
    res |= (((quint64)data[offset+1]) << 8);
    res |= (((quint64)data[offset+2]) << 16);
    res |= (((quint64)data[offset+3]) << 24);
    res |= (((quint64)data[offset+1]) << 32);
    res |= (((quint64)data[offset+2]) << 40);
    res |= (((quint64)data[offset+3]) << 48);
    res |= (((quint64)data[offset+3]) << 56);

    return res;
}

/*!
 \internal Reads an quint16 (2 bytes) from a byte array starting at given offset.
*/
quint16 UnzipPrivate::getUShort(const unsigned char* data, quint32 offset) const
{
    return (quint16) data[offset] | (((quint16)data[offset+1]) << 8);
}

/*!
 \internal Return the next byte in the pseudo-random sequence
 */
int UnzipPrivate::decryptByte(quint32 key2) const
{
    quint16 temp = ((quint16)(key2) & 0xffff) | 2;
    return (int)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

/*!
 \internal Update the encryption keys with the next byte of plain text
 */
void UnzipPrivate::updateKeys(quint32* keys, int c) const
{
    keys[0] = CRC32(keys[0], c);
    keys[1] += keys[0] & 0xff;
    keys[1] = keys[1] * 134775813L + 1;
    keys[2] = CRC32(keys[2], ((int)keys[1]) >> 24);
}

/*!
 \internal Initialize the encryption keys and the random header according to
 the given password.
 */
void UnzipPrivate::initKeys(const QString& pwd, quint32* keys) const
{
    keys[0] = 305419896L;
    keys[1] = 591751049L;
    keys[2] = 878082192L;

    QByteArray pwdBytes = pwd.toLatin1();
    int sz = pwdBytes.size();
    const char* ascii = pwdBytes.data();

    for (int i = 0; i < sz; ++i)
        updateKeys(keys, (int)ascii[i]);
}

/*!
 \internal Attempts to test a password without actually extracting a file.
 The \p file parameter can be used in the user interface or for debugging purposes
 as it is the name of the encrypted file for wich the password is being tested.
*/
UnZip::ErrorCode UnzipPrivate::testPassword(quint32* keys, const QString& file, const ZipEntryP& header)
{
    Q_UNUSED(file);
    Q_ASSERT(device);

    // read encryption keys
    if (device->read(buffer1, 12) != 12)
        return UnZip::Corrupted;

    // Replace this code if you want to i.e. call some dialog and ask the user for a password
    initKeys(password, keys);
    if (testKeys(header, keys))
        return UnZip::Ok;

    return UnZip::Skip;
}

/*!
 \internal Tests a set of keys on the encryption header.
*/
bool UnzipPrivate::testKeys(const ZipEntryP& header, quint32* keys)
{
    char lastByte;

    // decrypt encryption header
    for (int i = 0; i < 11; ++i)
        updateKeys(keys, lastByte = buffer1[i] ^ decryptByte(keys[2]));
    updateKeys(keys, lastByte = buffer1[11] ^ decryptByte(keys[2]));

    // if there is an extended header (bit in the gp flag) buffer[11] is a byte from the file time
    // with no extended header we have to check the crc high-order byte
    char c = ((header.gpFlag[0] & 0x08) == 8) ? header.modTime[1] : header.crc >> 24;

    return (lastByte == c);
}

/*!
 \internal Decrypts an array of bytes long \p read.
*/
void UnzipPrivate::decryptBytes(quint32* keys, char* buffer, qint64 read)
{
    for (int i = 0; i < (int)read; ++i)
        updateKeys(keys, buffer[i] ^= decryptByte(keys[2]));
}

/*!
 \internal Converts date and time values from ZIP format to a QDateTime object.
*/
QDateTime UnzipPrivate::convertDateTime(const unsigned char date[2], const unsigned char time[2]) const
{
    QDateTime dt;

    // Usual PKZip low-byte to high-byte order

    // Date: 7 bits = years from 1980, 4 bits = month, 5 bits = day
    quint16 year = (date[1] >> 1) & 127;
    quint16 month = ((date[1] << 3) & 14) | ((date[0] >> 5) & 7);
    quint16 day = date[0] & 31;

    // Time: 5 bits hour, 6 bits minutes, 5 bits seconds with a 2sec precision
    quint16 hour = (time[1] >> 3) & 31;
    quint16 minutes = ((time[1] << 3) & 56) | ((time[0] >> 5) & 7);
    quint16 seconds = (time[0] & 31) * 2;

    dt.setDate(QDate(1980 + year, month, day));
    dt.setTime(QTime(hour, minutes, seconds));
    return dt;
}


/************************************************************************
 Public interface
*************************************************************************/

/*!
 Creates a new Zip file decompressor.
*/
UnZip::UnZip() : d(new UnzipPrivate)
{
}

/*!
 Closes any open archive and releases used resources.
*/
UnZip::~UnZip()
{
    closeArchive();
    delete d;
}

/*!
 Returns true if there is an open archive.
*/
bool UnZip::isOpen() const
{
    return d->device;
}

/*!
 Opens a zip archive and reads the files list. Closes any previously opened archive.
*/
UnZip::ErrorCode UnZip::openArchive(const QString& filename)
{
    closeArchive();

    // closeArchive will destroy the file
    d->file = new QFile(filename);

    if (!d->file->exists()) {
        delete d->file;
        d->file = 0;
        return UnZip::FileNotFound;
    }

    if (!d->file->open(QIODevice::ReadOnly)) {
        delete d->file;
        d->file = 0;
        return UnZip::OpenFailed;
    }

    return d->openArchive(d->file);
}

/*!
 Opens a zip archive and reads the entries list.
 Closes any previously opened archive.
 \warning The class takes DOES NOT take ownership of the device.
*/
UnZip::ErrorCode UnZip::openArchive(QIODevice* device)
{
    closeArchive();

    if (!device) {
        qDebug() << "Invalid device.";
        return UnZip::InvalidDevice;
    }

    return d->openArchive(device);
}

/*!
 Closes the archive and releases all the used resources (like cached passwords).
*/
void UnZip::closeArchive()
{
    d->closeArchive();
}

QString UnZip::archiveComment() const
{
    return d->comment;
}

/*!
 Returns a locale translated error string for a given error code.
*/
QString UnZip::formatError(UnZip::ErrorCode c) const
{
    switch (c)
    {
    case Ok: return QCoreApplication::translate("UnZip", "ZIP operation completed successfully."); break;
    case ZlibInit: return QCoreApplication::translate("UnZip", "Failed to initialize or load zlib library."); break;
    case ZlibError: return QCoreApplication::translate("UnZip", "zlib library error."); break;
    case OpenFailed: return QCoreApplication::translate("UnZip", "Unable to create or open file."); break;
    case PartiallyCorrupted: return QCoreApplication::translate("UnZip", "Partially corrupted archive. Some files might be extracted."); break;
    case Corrupted: return QCoreApplication::translate("UnZip", "Corrupted archive."); break;
    case WrongPassword: return QCoreApplication::translate("UnZip", "Wrong password."); break;
    case NoOpenArchive: return QCoreApplication::translate("UnZip", "No archive has been created yet."); break;
    case FileNotFound: return QCoreApplication::translate("UnZip", "File or directory does not exist."); break;
    case ReadFailed: return QCoreApplication::translate("UnZip", "File read error."); break;
    case WriteFailed: return QCoreApplication::translate("UnZip", "File write error."); break;
    case SeekFailed: return QCoreApplication::translate("UnZip", "File seek error."); break;
    case CreateDirFailed: return QCoreApplication::translate("UnZip", "Unable to create a directory."); break;
    case InvalidDevice: return QCoreApplication::translate("UnZip", "Invalid device."); break;
    case InvalidArchive: return QCoreApplication::translate("UnZip", "Invalid or incompatible zip archive."); break;
    case HeaderConsistencyError: return QCoreApplication::translate("UnZip", "Inconsistent headers. Archive might be corrupted."); break;
    default: ;
    }

    return QCoreApplication::translate("UnZip", "Unknown error.");
}

/*!
 Returns true if the archive contains a file with the given path and name.
*/
bool UnZip::contains(const QString& file) const
{
    return d->headers ? d->headers->contains(file) : false;
}

/*!
 Returns complete paths of files and directories in this archive.
*/
QStringList UnZip::fileList() const
{
    return d->headers ? d->headers->keys() : QStringList();
}

/*!
 Returns information for each (correctly parsed) entry of this archive.
*/
QList<UnZip::ZipEntry> UnZip::entryList() const
{
    QList<UnZip::ZipEntry> list;
    if (!d->headers)
        return list;

    for (QMap<QString,ZipEntryP*>::ConstIterator it = d->headers->constBegin();
    it != d->headers->constEnd(); ++it) {
        const ZipEntryP* entry = it.value();
        Q_ASSERT(entry != 0);

        ZipEntry z;

        z.filename = it.key();
        if (!entry->comment.isEmpty())
            z.comment = entry->comment;
        z.compressedSize = entry->szComp;
        z.uncompressedSize = entry->szUncomp;
        z.crc32 = entry->crc;
        z.lastModified = d->convertDateTime(entry->modDate, entry->modTime);

        z.compression = entry->compMethod == 0 ? NoCompression : entry->compMethod == 8 ? Deflated : UnknownCompression;
        z.type = z.filename.endsWith("/") ? Directory : File;

        z.encrypted = entry->isEncrypted();

        list.append(z);
    }

    return list;
}

/*!
 Extracts the whole archive to a directory.
*/
UnZip::ErrorCode UnZip::verifyArchive()
{
    return extractAll(QDir(), VerifyOnly);
}

/*!
 Extracts the whole archive to a directory.
*/
UnZip::ErrorCode UnZip::extractAll(const QString& dirname, ExtractionOptions options)
{
    return extractAll(QDir(dirname), options);
}

/*!
 Extracts the whole archive to a directory.
 Stops extraction at the first error.
*/
UnZip::ErrorCode UnZip::extractAll(const QDir& dir, ExtractionOptions options)
{
    // this should only happen if we didn't call openArchive() yet
    if (!d->device)
        return NoOpenArchive;

    if (!d->headers)
        return Ok;

    ErrorCode ec = Ok;

    QMap<QString,ZipEntryP*>::ConstIterator it = d->headers->constBegin();
    const QMap<QString,ZipEntryP*>::ConstIterator end = d->headers->constEnd();
    while (it != end) {
        ZipEntryP* entry = it.value();
        Q_ASSERT(entry != 0);
        if ((entry->isEncrypted()) && d->skipAllEncrypted) {
            ++it;
            continue;
        }

        bool skip = false;
        ec = d->extractFile(it.key(), *entry, dir, options);
        switch (ec) {
        case Corrupted:
            qDebug() << "Corrupted entry" << it.key();
            break;
        case CreateDirFailed:
            break;
        case Skip:
            skip = true;
            break;
        case SkipAll:
            skip = true;
            d->skipAllEncrypted = true;
            break;
        default:
            ;
        }

        if (ec != Ok && !skip) {
            break;
        }

        ++it;
    }

    return ec;
}

/*!
 Extracts a single file to a directory.
*/
UnZip::ErrorCode UnZip::extractFile(const QString& filename, const QString& dirname, ExtractionOptions options)
{
    return extractFile(filename, QDir(dirname), options);
}

/*!
 Extracts a single file to a directory.
*/
UnZip::ErrorCode UnZip::extractFile(const QString& filename, const QDir& dir, ExtractionOptions options)
{
    if (!d->device)
        return NoOpenArchive;
    if (!d->headers)
        return FileNotFound;

    QMap<QString,ZipEntryP*>::Iterator itr = d->headers->find(filename);
    if (itr != d->headers->end()) {
        ZipEntryP* entry = itr.value();
        Q_ASSERT(entry != 0);
        return d->extractFile(itr.key(), *entry, dir, options);
    }

    return FileNotFound;
}

/*!
 Extracts a single file to a directory.
*/
UnZip::ErrorCode UnZip::extractFile(const QString& filename, QIODevice* outDev, ExtractionOptions options)
{
    if (!d->device)
        return NoOpenArchive;
    if (!d->headers)
        return FileNotFound;
    if (!outDev)
        return InvalidDevice;

    QMap<QString,ZipEntryP*>::Iterator itr = d->headers->find(filename);
    if (itr != d->headers->end()) {
        ZipEntryP* entry = itr.value();
        Q_ASSERT(entry != 0);
        return d->extractFile(itr.key(), *entry, outDev, options);
    }

    return FileNotFound;
}

/*!
 Extracts a list of files.
 Stops extraction at the first error (but continues if a file does not exist in the archive).
 */
UnZip::ErrorCode UnZip::extractFiles(const QStringList& filenames, const QString& dirname, ExtractionOptions options)
{
    if (!d->device)
        return NoOpenArchive;
    if (!d->headers)
        return Ok;

    QDir dir(dirname);
    ErrorCode ec;

    for (QStringList::ConstIterator itr = filenames.constBegin(); itr != filenames.constEnd(); ++itr) {
        ec = extractFile(*itr, dir, options);
        if (ec == FileNotFound)
            continue;
        if (ec != Ok)
            return ec;
    }

    return Ok;
}

/*!
 Extracts a list of files.
 Stops extraction at the first error (but continues if a file does not exist in the archive).
 */
UnZip::ErrorCode UnZip::extractFiles(const QStringList& filenames, const QDir& dir, ExtractionOptions options)
{
    if (!d->device)
        return NoOpenArchive;
    if (!d->headers)
        return Ok;

    ErrorCode ec;

    for (QStringList::ConstIterator itr = filenames.constBegin(); itr != filenames.constEnd(); ++itr) {
        ec = extractFile(*itr, dir, options);
        if (ec == FileNotFound)
            continue;
        if (ec != Ok)
            return ec;
    }

    return Ok;
}

/*!
 Remove/replace this method to add your own password retrieval routine.
*/
void UnZip::setPassword(const QString& pwd)
{
    d->password = pwd;
}

OSDAB_END_NAMESPACE
