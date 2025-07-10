/****************************************************************************
** Filename: zip.cpp
** Last updated [dd/mm/yyyy]: 01/02/2007
**
** pkzip 2.0 file compression.
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

#include "zip.h"
#include "zip_p.h"
#include "zipentry_p.h"

// we only use this to seed the random number generator
#include <ctime>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

// You can remove this #include if you replace the qDebug() statements.
#include <QtCore/QtDebug>


/*! #define OSDAB_ZIP_NO_PNG_RLE to disable the use of Z_RLE compression strategy with
    PNG files (achieves slightly better compression levels according to the authors).
*/
// #define OSDAB_ZIP_NO_PNG_RLE

#define OSDAB_ZIP_NO_DEBUG

//! Local header size (including signature, excluding variable length fields)
#define ZIP_LOCAL_HEADER_SIZE 30
//! Encryption header size
#define ZIP_LOCAL_ENC_HEADER_SIZE 12
//! Data descriptor size (signature included)
#define ZIP_DD_SIZE_WS 16
//! Central Directory record size (signature included)
#define ZIP_CD_SIZE 46
//! End of Central Directory record size (signature included)
#define ZIP_EOCD_SIZE 22

// Some offsets inside a local header record (signature included)
#define ZIP_LH_OFF_VERS 4
#define ZIP_LH_OFF_GPFLAG 6
#define ZIP_LH_OFF_CMET 8
#define ZIP_LH_OFF_MODT 10
#define ZIP_LH_OFF_MODD 12
#define ZIP_LH_OFF_CRC 14
#define ZIP_LH_OFF_CSIZE 18
#define ZIP_LH_OFF_USIZE 22
#define ZIP_LH_OFF_NAMELEN 26
#define ZIP_LH_OFF_XLEN 28

// Some offsets inside a data descriptor record (including signature)
#define ZIP_DD_OFF_CRC32 4
#define ZIP_DD_OFF_CSIZE 8
#define ZIP_DD_OFF_USIZE 12

// Some offsets inside a Central Directory record (including signature)
#define ZIP_CD_OFF_MADEBY 4
#define ZIP_CD_OFF_VERSION 6
#define ZIP_CD_OFF_GPFLAG 8
#define ZIP_CD_OFF_CMET 10
#define ZIP_CD_OFF_MODT 12
#define ZIP_CD_OFF_MODD 14
#define ZIP_CD_OFF_CRC 16
#define ZIP_CD_OFF_CSIZE 20
#define ZIP_CD_OFF_USIZE 24
#define ZIP_CD_OFF_NAMELEN 28
#define ZIP_CD_OFF_XLEN 30
#define ZIP_CD_OFF_COMMLEN 32
#define ZIP_CD_OFF_DISKSTART 34
#define ZIP_CD_OFF_IATTR 36
#define ZIP_CD_OFF_EATTR 38
#define ZIP_CD_OFF_LHOFF 42

// Some offsets inside a EOCD record (including signature)
#define ZIP_EOCD_OFF_DISKNUM 4
#define ZIP_EOCD_OFF_CDDISKNUM 6
#define ZIP_EOCD_OFF_ENTRIES 8
#define ZIP_EOCD_OFF_CDENTRIES 10
#define ZIP_EOCD_OFF_CDSIZE 12
#define ZIP_EOCD_OFF_CDOFF 16
#define ZIP_EOCD_OFF_COMMLEN 20

//! PKZip version for archives created by this API
#define ZIP_VERSION 0x14

//! Do not store very small files as the compression headers overhead would be to big
#define ZIP_COMPRESSION_THRESHOLD 60

/*!
	\class Zip zip.h

	\brief Zip file compression.

	Some quick usage examples.

	\verbatim
	Suppose you have this directory structure:

    /home/user/dir1/file1.1
    /home/user/dir1/file1.2
    /home/user/dir1/dir1.1/
    /home/user/dir1/dir1.2/file1.2.1

    EXAMPLE 1:
    myZipInstance.addDirectory("/home/user/dir1");

	RESULT:
	Beheaves like any common zip software and creates a zip file with this structure:

	dir1/file1.1
	dir1/file1.2
	dir1/dir1.1/
	dir1/dir1.2/file1.2.1

	EXAMPLE 2:
    myZipInstance.addDirectory("/home/user/dir1", "myRoot/myFolder");

	RESULT:
	Adds a custom root to the paths and creates a zip file with this structure:

	myRoot/myFolder/dir1/file1.1
	myRoot/myFolder/dir1/file1.2
	myRoot/myFolder/dir1/dir1.1/
	myRoot/myFolder/dir1/dir1.2/file1.2.1

	EXAMPLE 3:
    myZipInstance.addDirectory("/home/user/dir1", Zip::AbsolutePaths);

	NOTE:
	Same as calling addDirectory(SOME_PATH, PARENT_PATH_of_SOME_PATH).

	RESULT:
	Preserves absolute paths and creates a zip file with this structure:

    /home/user/dir1/file1.1
    /home/user/dir1/file1.2
    /home/user/dir1/dir1.1/
    /home/user/dir1/dir1.2/file1.2.1

	EXAMPLE 4:
	myZipInstance.setPassword("hellopass");
    myZipInstance.addDirectory("/home/user/dir1", "/");

	RESULT:
    Adds and encrypts the files in /home/user/dir1, creating the following zip structure:

	/dir1/file1.1
	/dir1/file1.2
	/dir1/dir1.1/
	/dir1/dir1.2/file1.2.1

    EXAMPLE 5:
    myZipInstance.addDirectory("/home/user/dir1", Zip::IgnoreRoot);

    RESULT:
    Adds the files in /home/user/dir1 but doesn't create the top level
    directory:

    file1.1
    file1.2
    dir1.1/
    dir1.2/file1.2.1

    EXAMPLE 5:
    myZipInstance.addDirectory("/home/user/dir1", "data/backup", Zip::IgnoreRoot);

    RESULT:
    Adds the files in /home/user/dir1 but uses "data/backup" as top level
    directory instead of "dir1":

    data/backup/file1.1
    data/backup/file1.2
    data/backup/dir1.1/
    data/backup/dir1.2/file1.2.1

	\endverbatim
*/

/*! \enum Zip::ErrorCode The result of a compression operation.
	\value Zip::Ok No error occurred.
	\value Zip::ZlibInit Failed to init or load the zlib library.
	\value Zip::ZlibError The zlib library returned some error.
	\value Zip::FileExists The file already exists and will not be overwritten.
	\value Zip::OpenFailed Unable to create or open a device.
	\value Zip::NoOpenArchive CreateArchive() has not been called yet.
	\value Zip::FileNotFound File or directory does not exist.
	\value Zip::ReadFailed Reading of a file failed.
	\value Zip::WriteFailed Writing of a file failed.
	\value Zip::SeekFailed Seek failed.
*/

/*! \enum Zip::CompressionLevel Returns the result of a decompression operation.
	\value Zip::Store No compression.
	\value Zip::Deflate1 Deflate compression level 1(lowest compression).
	\value Zip::Deflate1 Deflate compression level 2.
	\value Zip::Deflate1 Deflate compression level 3.
	\value Zip::Deflate1 Deflate compression level 4.
	\value Zip::Deflate1 Deflate compression level 5.
	\value Zip::Deflate1 Deflate compression level 6.
	\value Zip::Deflate1 Deflate compression level 7.
	\value Zip::Deflate1 Deflate compression level 8.
	\value Zip::Deflate1 Deflate compression level 9 (maximum compression).
	\value Zip::AutoCPU Adapt compression level to CPU speed (faster CPU => better compression).
	\value Zip::AutoMIME Adapt compression level to MIME type of the file being compressed.
	\value Zip::AutoFull Use both CPU and MIME type detection.
*/

namespace {

struct ZippedDir {
    bool init;
    QString actualRoot;
    int files;
    ZippedDir() : init(false), actualRoot(), files(0) {}
};

void checkRootPath(QString& path)
{
    const bool isUnixRoot = path.length() == 1 && path.at(0) == QLatin1Char('/');
    if (!path.isEmpty() && !isUnixRoot) {
        while (path.endsWith(QLatin1String("\\")))
            path.truncate(path.length() - 1);

        int sepCount = 0;
        for (int i = path.length()-1; i >= 0; --i) {
            if (path.at(i) == QLatin1Char('/'))
                ++sepCount;
            else break;
        }

        if (sepCount > 1)
            path.truncate(path.length() - (sepCount-1));
        else if (sepCount == 0)
            path.append(QLatin1String("/"));
    }
}

}

//////////////////////////////////////////////////////////////////////////

OSDAB_BEGIN_NAMESPACE(Zip)

/************************************************************************
 Private interface
*************************************************************************/

//! \internal
ZipPrivate::ZipPrivate() :
    headers(0),
    device(0),
    file(0),
    uBuffer(0),
    crcTable(0),
    comment(),
    password()
{
	// keep an unsigned pointer so we avoid to over bloat the code with casts
	uBuffer = (unsigned char*) buffer1;
    crcTable = get_crc_table();
}

//! \internal
ZipPrivate::~ZipPrivate()
{
	closeArchive();
}

//! \internal
Zip::ErrorCode ZipPrivate::createArchive(QIODevice* dev)
{
	Q_ASSERT(dev);

	if (device)
		closeArchive();

	device = dev;
    if (device != file)
        connect(device, SIGNAL(destroyed(QObject*)), this, SLOT(deviceDestroyed(QObject*)));

	if (!device->isOpen()) {
		if (!device->open(QIODevice::ReadOnly)) {
			delete device;
			device = 0;
			qDebug() << "Unable to open device for writing.";
			return Zip::OpenFailed;
		}
	}

	headers = new QMap<QString,ZipEntryP*>;
	return Zip::Ok;
}

//! \internal
void ZipPrivate::deviceDestroyed(QObject*)
{
    qDebug("Unexpected device destruction detected.");
    do_closeArchive();
}

/*! Returns true if an entry for \p info has already been added.
    Uses file size and lower case absolute path to compare entries.
*/
bool ZipPrivate::containsEntry(const QFileInfo& info) const
{
    if (!headers || headers->isEmpty())
        return false;

    const qint64 sz = info.size();
    const QString path = info.absoluteFilePath().toLower();

    QMap<QString,ZipEntryP*>::ConstIterator b = headers->constBegin();
    const QMap<QString,ZipEntryP*>::ConstIterator e = headers->constEnd();
    while (b != e) {
        const ZipEntryP* e = b.value();
        if (e->fileSize == sz && e->absolutePath == path)
            return true;
        ++b;
    }

    return false;
}

//! \internal Actual implementation of the addDirectory* methods.
Zip::ErrorCode ZipPrivate::addDirectory(const QString& path, const QString& root,
    Zip::CompressionOptions options, Zip::CompressionLevel level, int hierarchyLevel,
    int* addedFiles)
{
    if (addedFiles)
        ++(*addedFiles);

    // Bad boy didn't call createArchive() yet :)
    if (!device)
        return Zip::NoOpenArchive;

    QDir dir(path);
    if (!dir.exists())
        return Zip::FileNotFound;

    // Remove any trailing separator
    QString actualRoot = root.trimmed();

    // Preserve Unix root but make sure the path ends only with a single
    // unix like separator
    ::checkRootPath(actualRoot);

    // QDir::cleanPath() fixes some issues with QDir::dirName()
    QFileInfo current(QDir::cleanPath(path));

    const bool path_absolute = options.testFlag(Zip::AbsolutePaths);
    const bool path_ignore = options.testFlag(Zip::IgnorePaths);
    const bool path_noroot = options.testFlag(Zip::IgnoreRoot);

    if (path_absolute && !path_ignore && !path_noroot) {
        QString absolutePath = extractRoot(path, options);
        if (!absolutePath.isEmpty() && absolutePath != QLatin1String("/"))
            absolutePath.append(QLatin1String("/"));
        actualRoot.append(absolutePath);
    }

    const bool skipDirName = !hierarchyLevel && path_noroot;
    if (!path_ignore && !skipDirName) {
        actualRoot.append(QDir(current.absoluteFilePath()).dirName());
        actualRoot.append(QLatin1String("/"));
    }

    // actualRoot now contains the path of the file relative to the zip archive
    // with a trailing /

    const bool skipBad = options & Zip::SkipBadFiles;
    const bool noDups = options & Zip::CheckForDuplicates;

    const QDir::Filters dir_filter =
            QDir::Files |
            QDir::Dirs |
            QDir::NoDotAndDotDot |
            QDir::NoSymLinks;
    const QDir::SortFlags dir_sort =
            QDir::DirsFirst;
    QFileInfoList list = dir.entryInfoList(dir_filter, dir_sort);

    Zip::ErrorCode ec = Zip::Ok;
    bool filesAdded = false;

    Zip::CompressionOptions recursionOptions;
    if (path_ignore)
        recursionOptions |= Zip::IgnorePaths;
    else recursionOptions |= Zip::RelativePaths;

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo info = list.at(i);
        const QString absPath = info.absoluteFilePath();
        if (noDups && containsEntry(info))
            continue;
        if (info.isDir()) {
            // Recursion
            ec = addDirectory(absPath, actualRoot, recursionOptions,
                level, hierarchyLevel + 1, addedFiles);
        } else {
            ec = createEntry(info, actualRoot, level);
            if (ec == Zip::Ok) {
                filesAdded = true;
                if (addedFiles)
                    ++(*addedFiles);
            }
        }

        if (ec != Zip::Ok && !skipBad) {
           break;
        }
    }

    // We need an explicit record for this dir
    // Non-empty directories don't need it because they have a path component in the filename
    if (!filesAdded && !path_ignore)
        ec = createEntry(current, actualRoot, level);

    return ec;
}

//! \internal Actual implementation of the addFile methods.
Zip::ErrorCode ZipPrivate::addFiles(const QStringList& files, const QString& root,
    Zip::CompressionOptions options, Zip::CompressionLevel level,
    int* addedFiles)
{
    if (addedFiles)
        *addedFiles = 0;

    const bool skipBad = options & Zip::SkipBadFiles;
    const bool noDups = options & Zip::CheckForDuplicates;

    // Bad boy didn't call createArchive() yet :)
    if (!device)
        return Zip::NoOpenArchive;

    QFileInfoList paths;
    paths.reserve(files.size());
    for (int i = 0; i < files.size(); ++i) {
        QFileInfo info(files.at(i));
        if (noDups && (paths.contains(info) || containsEntry(info)))
            continue;
        if (!info.exists() || !info.isReadable()) {
            if (skipBad) {
                continue;
            } else {
                return Zip::FileNotFound;
            }
        }
        paths.append(info);
    }

    if (paths.isEmpty())
        return Zip::Ok;

    // Remove any trailing separator
    QString actualRoot = root.trimmed();

    // Preserve Unix root but make sure the path ends only with a single
    // unix like separator
    ::checkRootPath(actualRoot);

    const bool path_absolute = options.testFlag(Zip::AbsolutePaths);
    const bool path_ignore = options.testFlag(Zip::IgnorePaths);
    const bool path_noroot = options.testFlag(Zip::IgnoreRoot);

    Zip::ErrorCode ec = Zip::Ok;
    QHash<QString, ZippedDir> dirMap;

    for (int i = 0; i < paths.size(); ++i) {
        const QFileInfo& info = paths.at(i);
        const QString path = QFileInfo(QDir::cleanPath(info.absolutePath())).absolutePath();

        ZippedDir& zd = dirMap[path];
        if (!zd.init) {
            zd.init = true;
            zd.actualRoot = actualRoot;
            if (path_absolute && !path_ignore && !path_noroot) {
                QString absolutePath = extractRoot(path, options);
                if (!absolutePath.isEmpty() && absolutePath != QLatin1String("/"))
                    absolutePath.append(QLatin1String("/"));
                zd.actualRoot.append(absolutePath);
            }

            if (!path_ignore && !path_noroot) {
                zd.actualRoot.append(QDir(path).dirName());
                zd.actualRoot.append(QLatin1String("/"));
            }
        }

        // zd.actualRoot now contains the path of the file relative to the zip archive
        // with a trailing /

        if (info.isDir()) {
            // Recursion
            ec = addDirectory(info.absoluteFilePath(), actualRoot, options,
                level, 1, addedFiles);
        } else {
            ec = createEntry(info, actualRoot, level);
            if (ec == Zip::Ok) {
                ++zd.files;
                if (addedFiles)
                    ++(*addedFiles);
            }
        }

        if (ec != Zip::Ok && !skipBad) {
           break;
        }
    }

    // Create explicit records for empty directories
    if (!path_ignore) {
        QHash<QString, ZippedDir>::ConstIterator b = dirMap.constBegin();
        const QHash<QString, ZippedDir>::ConstIterator e = dirMap.constEnd();
        while (b != e) {
            const ZippedDir& zd = b.value();
            if (zd.files <= 0) {
                ec = createEntry(b.key(), zd.actualRoot, level);
            }
            ++b;
        }
    }

    return ec;
}

//! \internal \p file must be a file and not a directory.
Zip::ErrorCode ZipPrivate::deflateFile(const QFileInfo& fileInfo,
    quint32& crc, qint64& written, const Zip::CompressionLevel& level, quint32** keys)
{
    const QString path = fileInfo.absoluteFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << QString("An error occurred while opening %1").arg(path);
        return Zip::OpenFailed;
    }

    const Zip::ErrorCode ec = (level == Zip::Store)
        ? storeFile(path, file, crc, written, keys)
        : compressFile(path, file, crc, written, level, keys);

    file.close();
    return ec;
}

//! \internal
Zip::ErrorCode ZipPrivate::storeFile(const QString& path, QIODevice& file,
    quint32& crc, qint64& totalWritten, quint32** keys)
{
    Q_UNUSED(path);

    qint64 read = 0;
    qint64 written = 0;

    const bool encrypt = keys != 0;

    totalWritten = 0;
    crc = crc32(0L, Z_NULL, 0);

    while ( (read = file.read(buffer1, ZIP_READ_BUFFER)) > 0 ) {
        crc = crc32(crc, uBuffer, read);
        if (encrypt)
            encryptBytes(*keys, buffer1, read);
        written = device->write(buffer1, read);
        totalWritten += written;
        if (written != read) {
            return Zip::WriteFailed;
        }
    }

    return Zip::Ok;
}

//! \internal
int ZipPrivate::compressionStrategy(const QString& path, QIODevice& file) const
{
    Q_UNUSED(file);

#ifndef OSDAB_ZIP_NO_PNG_RLE
    return Z_DEFAULT_STRATEGY;
#endif
    const bool isPng = path.endsWith(QLatin1String("png"), Qt::CaseInsensitive);
    return isPng ? Z_RLE : Z_DEFAULT_STRATEGY;
}

//! \internal
Zip::ErrorCode ZipPrivate::compressFile(const QString& path, QIODevice& file,
    quint32& crc, qint64& totalWritten, const Zip::CompressionLevel& level, quint32** keys)
{
    qint64 read = 0;
    qint64 written = 0;

    qint64 totRead = 0;
    qint64 toRead = file.size();

    const bool encrypt = keys != 0;
    const int strategy = compressionStrategy(path, file);

    totalWritten = 0;
    crc = crc32(0L, Z_NULL, 0);

    z_stream zstr;

    // Initialize zalloc, zfree and opaque before calling the init function
    zstr.zalloc = Z_NULL;
    zstr.zfree = Z_NULL;
    zstr.opaque = Z_NULL;

    int zret;

    // Use deflateInit2 with negative windowBits to get raw compression
    if ((zret = deflateInit2_(
            &zstr,
            (int)level, // compression level
            Z_DEFLATED, // method
            -MAX_WBITS, // windowBits
            8, // memLevel
            strategy,
            ZLIB_VERSION,
            sizeof(z_stream)
        )) != Z_OK ) {
        qDebug() << "Could not initialize zlib for compression";
        return Zip::ZlibError;
    }

    qint64 compressed;
    int flush = Z_NO_FLUSH;
    do {
        read = file.read(buffer1, ZIP_READ_BUFFER);
        totRead += read;
        if (!read)
            break;

        if (read < 0) {
            deflateEnd(&zstr);
            qDebug() << QString("Error while reading %1").arg(path);
            return Zip::ReadFailed;
        }

        crc = crc32(crc, uBuffer, read);

        zstr.next_in = (Bytef*) buffer1;
        zstr.avail_in = (uInt)read;

        // Tell zlib if this is the last chunk we want to encode
        // by setting the flush parameter to Z_FINISH
        flush = (totRead == toRead) ? Z_FINISH : Z_NO_FLUSH;

        // Run deflate() on input until output buffer not full
        // finish compression if all of source has been read in
        do {
            zstr.next_out = (Bytef*) buffer2;
            zstr.avail_out = ZIP_READ_BUFFER;

            zret = deflate(&zstr, flush);
            // State not clobbered
            Q_ASSERT(zret != Z_STREAM_ERROR);

            // Write compressed data to file and empty buffer
            compressed = ZIP_READ_BUFFER - zstr.avail_out;

            if (encrypt)
                encryptBytes(*keys, buffer2, compressed);

            written = device->write(buffer2, compressed);
            totalWritten += written;

            if (written != compressed) {
                deflateEnd(&zstr);
                qDebug() << QString("Error while writing %1").arg(path);
                return Zip::WriteFailed;
            }

        } while (zstr.avail_out == 0);

        // All input will be used
        Q_ASSERT(zstr.avail_in == 0);

    } while (flush != Z_FINISH);

    // Stream will be complete
    Q_ASSERT(zret == Z_STREAM_END);
    deflateEnd(&zstr);

    return Zip::Ok;
}

//! \internal Writes a new entry in the zip file.
Zip::ErrorCode ZipPrivate::createEntry(const QFileInfo& file, const QString& root,
    Zip::CompressionLevel level)
{
    const bool dirOnly = file.isDir();

    // entryName contains the path as it should be written
    // in the zip file records
    const QString entryName = dirOnly
        ? root
        : root + file.fileName();

    // Directory entry
    if (dirOnly || file.size() < ZIP_COMPRESSION_THRESHOLD) {
		level = Zip::Store;
    } else {
        switch (level) {
        case Zip::AutoCPU:
            level = Zip::Deflate5;
#ifndef OSDAB_ZIP_NO_DEBUG
            qDebug("Compression level for '%s': %d", entryName.toLatin1().constData(), (int)level);
#endif
            break;
        case Zip::AutoMIME:
            level = detectCompressionByMime(file.completeSuffix().toLower());
#ifndef OSDAB_ZIP_NO_DEBUG
            qDebug("Compression level for '%s': %d", entryName.toLatin1().constData(), (int)level);
#endif
            break;
        case Zip::AutoFull:
            level = detectCompressionByMime(file.completeSuffix().toLower());
#ifndef OSDAB_ZIP_NO_DEBUG
            qDebug("Compression level for '%s': %d", entryName.toLatin1().constData(), (int)level);
#endif
            break;
        default: ;
        }
    }



	// create header and store it to write a central directory later
    QScopedPointer<ZipEntryP> h(new ZipEntryP);
    h->absolutePath = file.absoluteFilePath().toLower();
    h->fileSize = file.size();

    // Set encryption bit and set the data descriptor bit
	// so we can use mod time instead of crc for password check
	bool encrypt = !dirOnly && !password.isEmpty();
	if (encrypt)
		h->gpFlag[0] |= 9;

    QDateTime dt = file.lastModified();
    dt = OSDAB_ZIP_MANGLE(fromFileTimestamp)(dt);
	QDate d = dt.date();
	h->modDate[1] = ((d.year() - 1980) << 1) & 254;
	h->modDate[1] |= ((d.month() >> 3) & 1);
	h->modDate[0] = ((d.month() & 7) << 5) & 224;
	h->modDate[0] |= d.day();

	QTime t = dt.time();
	h->modTime[1] = (t.hour() << 3) & 248;
	h->modTime[1] |= ((t.minute() >> 3) & 7);
	h->modTime[0] = ((t.minute() & 7) << 5) & 224;
	h->modTime[0] |= t.second() / 2;

	h->szUncomp = dirOnly ? 0 : file.size();

    h->compMethod = (level == Zip::Store) ? 0 : 0x0008;

	// **** Write local file header ****

	// signature
	buffer1[0] = 'P'; buffer1[1] = 'K';
	buffer1[2] = 0x3; buffer1[3] = 0x4;

	// version needed to extract
	buffer1[ZIP_LH_OFF_VERS] = ZIP_VERSION;
	buffer1[ZIP_LH_OFF_VERS + 1] = 0;

	// general purpose flag
	buffer1[ZIP_LH_OFF_GPFLAG] = h->gpFlag[0];
	buffer1[ZIP_LH_OFF_GPFLAG + 1] = h->gpFlag[1];

	// compression method
	buffer1[ZIP_LH_OFF_CMET] = h->compMethod & 0xFF;
	buffer1[ZIP_LH_OFF_CMET + 1] = (h->compMethod>>8) & 0xFF;

	// last mod file time
	buffer1[ZIP_LH_OFF_MODT] = h->modTime[0];
	buffer1[ZIP_LH_OFF_MODT + 1] = h->modTime[1];

	// last mod file date
	buffer1[ZIP_LH_OFF_MODD] = h->modDate[0];
	buffer1[ZIP_LH_OFF_MODD + 1] = h->modDate[1];

	// skip crc (4bytes) [14,15,16,17]

	// skip compressed size but include evtl. encryption header (4bytes: [18,19,20,21])
	buffer1[ZIP_LH_OFF_CSIZE] =
	buffer1[ZIP_LH_OFF_CSIZE + 1] =
	buffer1[ZIP_LH_OFF_CSIZE + 2] =
	buffer1[ZIP_LH_OFF_CSIZE + 3] = 0;

	h->szComp = encrypt ? ZIP_LOCAL_ENC_HEADER_SIZE : 0;

	// uncompressed size [22,23,24,25]
	setULong(h->szUncomp, buffer1, ZIP_LH_OFF_USIZE);

	// filename length
	QByteArray entryNameBytes = entryName.toLatin1();
	int sz = entryNameBytes.size();

	buffer1[ZIP_LH_OFF_NAMELEN] = sz & 0xFF;
	buffer1[ZIP_LH_OFF_NAMELEN + 1] = (sz >> 8) & 0xFF;

	// extra field length
	buffer1[ZIP_LH_OFF_XLEN] = buffer1[ZIP_LH_OFF_XLEN + 1] = 0;

	// Store offset to write crc and compressed size
	h->lhOffset = device->pos();
	quint32 crcOffset = h->lhOffset + ZIP_LH_OFF_CRC;

	if (device->write(buffer1, ZIP_LOCAL_HEADER_SIZE) != ZIP_LOCAL_HEADER_SIZE) {
        return Zip::WriteFailed;
	}

	// Write out filename
	if (device->write(entryNameBytes) != sz) {
        return Zip::WriteFailed;
	}

	// Encryption keys
	quint32 keys[3] = { 0, 0, 0 };

	if (encrypt) {
		// **** encryption header ****

		// XOR with PI to ensure better random numbers
		// with poorly implemented rand() as suggested by Info-Zip
		srand(time(NULL) ^ 3141592654UL);
		int randByte;

		initKeys(keys);
		for (int i = 0; i < 10; ++i) {
			randByte = (rand() >> 7) & 0xff;
			buffer1[i] = decryptByte(keys[2]) ^ randByte;
			updateKeys(keys, randByte);
		}

		// Encrypt encryption header
		initKeys(keys);
		for (int i = 0; i < 10; ++i) {
			randByte = decryptByte(keys[2]);
			updateKeys(keys, buffer1[i]);
			buffer1[i] ^= randByte;
		}

		// We don't know the CRC at this time, so we use the modification time
		// as the last two bytes
		randByte = decryptByte(keys[2]);
		updateKeys(keys, h->modTime[0]);
		buffer1[10] ^= randByte;

		randByte = decryptByte(keys[2]);
		updateKeys(keys, h->modTime[1]);
		buffer1[11] ^= randByte;

		// Write out encryption header
		if (device->write(buffer1, ZIP_LOCAL_ENC_HEADER_SIZE) != ZIP_LOCAL_ENC_HEADER_SIZE) {
            return Zip::WriteFailed;
		}
	}

    quint32 crc = 0;
    qint64 written = 0;

    if (!dirOnly) {
        quint32* k = keys;
        const Zip::ErrorCode ec = deflateFile(file, crc, written, level, encrypt ? &k : 0);
        if (ec != Zip::Ok)
            return ec;
        Q_ASSERT(!h.isNull());
	}

	// Store end of entry offset
	quint32 current = device->pos();

	// Update crc and compressed size in local header
	if (!device->seek(crcOffset)) {
        return Zip::SeekFailed;
	}

	h->crc = dirOnly ? 0 : crc;
	h->szComp += written;

	setULong(h->crc, buffer1, 0);
	setULong(h->szComp, buffer1, 4);
	if ( device->write(buffer1, 8) != 8) {
        return Zip::WriteFailed;
	}

	// Seek to end of entry
    if (!device->seek(current)) {
		return Zip::SeekFailed;
	}

	if ((h->gpFlag[0] & 8) == 8) {
		// Write data descriptor

		// Signature: PK\7\8
		buffer1[0] = 'P';
		buffer1[1] = 'K';
		buffer1[2] = 0x07;
		buffer1[3] = 0x08;

		// CRC
		setULong(h->crc, buffer1, ZIP_DD_OFF_CRC32);

		// Compressed size
		setULong(h->szComp, buffer1, ZIP_DD_OFF_CSIZE);

		// Uncompressed size
		setULong(h->szUncomp, buffer1, ZIP_DD_OFF_USIZE);

        if (device->write(buffer1, ZIP_DD_SIZE_WS) != ZIP_DD_SIZE_WS) {
			return Zip::WriteFailed;
		}
	}

    headers->insert(entryName, h.take());
	return Zip::Ok;
}

//! \internal
int ZipPrivate::decryptByte(quint32 key2) const
{
    quint16 temp = ((quint16)(key2) & 0xffff) | 2;
	return (int)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

//! \internal Writes an quint32 (4 bytes) to a byte array at given offset.
void ZipPrivate::setULong(quint32 v, char* buffer, unsigned int offset)
{
	buffer[offset+3] = ((v >> 24) & 0xFF);
	buffer[offset+2] = ((v >> 16) & 0xFF);
	buffer[offset+1] = ((v >> 8) & 0xFF);
	buffer[offset] = (v & 0xFF);
}

//! \internal Initializes decryption keys using a password.
void ZipPrivate::initKeys(quint32* keys) const
{
	// Encryption keys initialization constants are taken from the
	// PKZip file format specification docs
	keys[0] = 305419896L;
	keys[1] = 591751049L;
	keys[2] = 878082192L;

	QByteArray pwdBytes = password.toLatin1();
	int sz = pwdBytes.size();
	const char* ascii = pwdBytes.data();

	for (int i = 0; i < sz; ++i)
		updateKeys(keys, (int)ascii[i]);
}

//! Updates a one-char-only CRC; it's the Info-Zip macro re-adapted.
quint32 ZipPrivate::updateChecksum(const quint32& crc, const quint32& val) const
{
    return quint32(crcTable[quint32(crc^val) & 0xff] ^ crc_t(crc >> 8));
}

//! \internal Updates encryption keys.
void ZipPrivate::updateKeys(quint32* keys, int c) const
{
    keys[0] = updateChecksum(keys[0], c);
	keys[1] += keys[0] & 0xff;
	keys[1] = keys[1] * 134775813L + 1;
    keys[2] = updateChecksum(keys[2], ((int)keys[1]) >> 24);
}

//! \internal Encrypts a byte array.
void ZipPrivate::encryptBytes(quint32* keys, char* buffer, qint64 read)
{
	char t;

    for (qint64 i = 0; i < read; ++i) {
		t = buffer[i];
		buffer[i] ^= decryptByte(keys[2]);
		updateKeys(keys, t);
	}
}

namespace {
struct KeywordHelper {
    const QString needle;
    inline KeywordHelper(const QString& keyword) : needle(keyword) {}
};

bool operator<(const KeywordHelper& helper, const char* keyword) {
    return helper.needle.compare(QLatin1String(keyword)) < 0;
}

bool operator<(const char* keyword, const KeywordHelper& helper) {
    return helper.needle.compare(QLatin1String(keyword)) > 0;
}

bool hasExtension(const QString& ext, const char* const* map, int max) {
    const char* const* start = &map[0];
    const char* const* end = &map[max - 1];
    const char* const* kw = qBinaryFind(start, end, KeywordHelper(ext));
    return kw != end;
}
}

//! \internal Detects the best compression level for a given file extension.
Zip::CompressionLevel ZipPrivate::detectCompressionByMime(const QString& ext)
{
    // NOTE: Keep the  MAX_* and the number of strings in the map up to date.
    // NOTE: Alphabetically sort the strings in the map -- we use a binary search!

    // Archives or files that will hardly compress
    const int MAX_EXT1 = 14;
    const char* const ext1[MAX_EXT1] = {
        "7z", "bin", "deb", "exe", "gz", "gz2", "jar", "rar", "rpm", "tar", "tgz", "z", "zip",
        0 // # MAX_EXT1
    };

    // Slow or usually large files that we should not spend to much time with
    const int MAX_EXT2 = 24;
    const char* const ext2[MAX_EXT2] = {
        "asf",
        "avi",
        "divx",
        "doc",
        "docx",
        "flv",
        "gif",
        "iso",
        "jpg",
        "jpeg",
        "mka",
        "mkv",
        "mp3",
        "mp4",
        "mpeg",
        "mpg",
        "odt",
        "ogg",
        "ogm",
        "ra",
        "rm",
        "wma",
        "wmv",
        0 // # MAX_EXT2
    };

    // Files with high compression ratio
    const int MAX_EXT3 = 28;
    const char* const ext3[MAX_EXT3] = {
        "asp", "bat", "c", "conf", "cpp", "cpp", "css", "csv", "cxx", "h", "hpp", "htm", "html", "hxx",
        "ini", "js", "php", "pl", "py", "rtf", "sh", "tsv", "txt", "vb", "vbs", "xml", "xst",
        0 // # MAX_EXT3
    };

    const char* const* map = ext1;
    if (hasExtension(ext, map, MAX_EXT1))
        return Zip::Store;

    map = ext2;
    if (hasExtension(ext, map, MAX_EXT2))
        return Zip::Deflate2;

    map = ext3;
    if (hasExtension(ext, map, MAX_EXT3))
        return Zip::Deflate9;

    return Zip::Deflate5;
}

/*!
	Closes the current archive and writes out pending data.
*/
Zip::ErrorCode ZipPrivate::closeArchive()
{
    if (!device) {
        Q_ASSERT(!file);
        return Zip::Ok;
    }

    if (device != file)
        disconnect(device, 0, this, 0);

    return do_closeArchive();
}

//! \internal
Zip::ErrorCode ZipPrivate::do_closeArchive()
{
	// Close current archive by writing out central directory
	// and free up resources

	if (!device && !headers)
		return Zip::Ok;

	quint32 szCentralDir = 0;
    quint32 offCentralDir = device->pos();
	Zip::ErrorCode c = Zip::Ok;

    if (headers && device) {
        for (QMap<QString,ZipEntryP*>::ConstIterator itr = headers->constBegin(); 
            itr != headers->constEnd(); ++itr) {
            const QString fileName = itr.key();
            const ZipEntryP* h = itr.value();
            c = writeEntry(fileName, h, szCentralDir);
        }
    }

    if (c == Zip::Ok)
        c = writeCentralDir(offCentralDir, szCentralDir);

    if (c != Zip::Ok) {
        if (file) {
            file->close();
            if (!file->remove()) {
                qDebug() << "Failed to delete corrupt archive.";
            }
        }
    }

	return c;
}

//! \internal
Zip::ErrorCode ZipPrivate::writeEntry(const QString& fileName, const ZipEntryP* h, quint32& szCentralDir)
{
    unsigned int sz;

    Q_ASSERT(h && device && headers);
		
    // signature
	buffer1[0] = 'P';
	buffer1[1] = 'K';
	buffer1[2] = 0x01;
	buffer1[3] = 0x02;

	// version made by  (currently only MS-DOS/FAT - no symlinks or other stuff supported)
	buffer1[ZIP_CD_OFF_MADEBY] = buffer1[ZIP_CD_OFF_MADEBY + 1] = 0;

	// version needed to extract
	buffer1[ZIP_CD_OFF_VERSION] = ZIP_VERSION;
	buffer1[ZIP_CD_OFF_VERSION + 1] = 0;

	// general purpose flag
	buffer1[ZIP_CD_OFF_GPFLAG] = h->gpFlag[0];
	buffer1[ZIP_CD_OFF_GPFLAG + 1] = h->gpFlag[1];

	// compression method
	buffer1[ZIP_CD_OFF_CMET] = h->compMethod & 0xFF;
	buffer1[ZIP_CD_OFF_CMET + 1] = (h->compMethod >> 8) & 0xFF;

	// last mod file time
	buffer1[ZIP_CD_OFF_MODT] = h->modTime[0];
	buffer1[ZIP_CD_OFF_MODT + 1] = h->modTime[1];

	// last mod file date
	buffer1[ZIP_CD_OFF_MODD] = h->modDate[0];
	buffer1[ZIP_CD_OFF_MODD + 1] = h->modDate[1];

	// crc (4bytes) [16,17,18,19]
	setULong(h->crc, buffer1, ZIP_CD_OFF_CRC);

	// compressed size (4bytes: [20,21,22,23])
	setULong(h->szComp, buffer1, ZIP_CD_OFF_CSIZE);

	// uncompressed size [24,25,26,27]
	setULong(h->szUncomp, buffer1, ZIP_CD_OFF_USIZE);

	// filename
	QByteArray fileNameBytes = fileName.toLatin1();
	sz = fileNameBytes.size();
	buffer1[ZIP_CD_OFF_NAMELEN] = sz & 0xFF;
	buffer1[ZIP_CD_OFF_NAMELEN + 1] = (sz >> 8) & 0xFF;

	// extra field length
	buffer1[ZIP_CD_OFF_XLEN] = buffer1[ZIP_CD_OFF_XLEN + 1] = 0;

	// file comment length
	buffer1[ZIP_CD_OFF_COMMLEN] = buffer1[ZIP_CD_OFF_COMMLEN + 1] = 0;

	// disk number start
	buffer1[ZIP_CD_OFF_DISKSTART] = buffer1[ZIP_CD_OFF_DISKSTART + 1] = 0;

	// internal file attributes
	buffer1[ZIP_CD_OFF_IATTR] = buffer1[ZIP_CD_OFF_IATTR + 1] = 0;

	// external file attributes
	buffer1[ZIP_CD_OFF_EATTR] =
	buffer1[ZIP_CD_OFF_EATTR + 1] =
	buffer1[ZIP_CD_OFF_EATTR + 2] =
	buffer1[ZIP_CD_OFF_EATTR + 3] = 0;

	// relative offset of local header [42->45]
	setULong(h->lhOffset, buffer1, ZIP_CD_OFF_LHOFF);

	if (device->write(buffer1, ZIP_CD_SIZE) != ZIP_CD_SIZE) {
		return Zip::WriteFailed;
	}

	// Write out filename
	if ((unsigned int)device->write(fileNameBytes) != sz) {
		return Zip::WriteFailed;
	}

	szCentralDir += (ZIP_CD_SIZE + sz);

    return Zip::Ok;
}

//! \internal
Zip::ErrorCode ZipPrivate::writeCentralDir(quint32 offCentralDir, quint32 szCentralDir)
{
    Q_ASSERT(device && headers);

    unsigned int sz;
	
    // signature
	buffer1[0] = 'P';
	buffer1[1] = 'K';
	buffer1[2] = 0x05;
	buffer1[3] = 0x06;

	// number of this disk
	buffer1[ZIP_EOCD_OFF_DISKNUM] = buffer1[ZIP_EOCD_OFF_DISKNUM + 1] = 0;

	// number of disk with central directory
	buffer1[ZIP_EOCD_OFF_CDDISKNUM] = buffer1[ZIP_EOCD_OFF_CDDISKNUM + 1] = 0;

	// number of entries in this disk
	sz = headers->count();
    buffer1[ZIP_EOCD_OFF_ENTRIES] = sz & 0xFF;
	buffer1[ZIP_EOCD_OFF_ENTRIES + 1] = (sz >> 8) & 0xFF;

	// total number of entries
	buffer1[ZIP_EOCD_OFF_CDENTRIES] = buffer1[ZIP_EOCD_OFF_ENTRIES];
	buffer1[ZIP_EOCD_OFF_CDENTRIES + 1] = buffer1[ZIP_EOCD_OFF_ENTRIES + 1];

	// size of central directory [12->15]
	setULong(szCentralDir, buffer1, ZIP_EOCD_OFF_CDSIZE);

	// central dir offset [16->19]
	setULong(offCentralDir, buffer1, ZIP_EOCD_OFF_CDOFF);

	// ZIP file comment length
	QByteArray commentBytes = comment.toLatin1();
	quint16 commentLength = commentBytes.size();

	if (commentLength == 0) {
		buffer1[ZIP_EOCD_OFF_COMMLEN] = buffer1[ZIP_EOCD_OFF_COMMLEN + 1] = 0;
	} else {
		buffer1[ZIP_EOCD_OFF_COMMLEN] = commentLength & 0xFF;
		buffer1[ZIP_EOCD_OFF_COMMLEN + 1] = (commentLength >> 8) & 0xFF;
	}

	if (device->write(buffer1, ZIP_EOCD_SIZE) != ZIP_EOCD_SIZE) {
		return Zip::WriteFailed;
	}

	if (commentLength != 0) {
		if ((unsigned int)device->write(commentBytes) != commentLength) {
			return Zip::WriteFailed;
		}
	}

    return Zip::Ok;
}

//! \internal
void ZipPrivate::reset()
{
	comment.clear();

	if (headers) {
		qDeleteAll(*headers);
		delete headers;
		headers = 0;
	}

	device = 0;

    if (file)
        delete file;
    file = 0;
}

//! \internal Returns the path of the parent directory
QString ZipPrivate::extractRoot(const QString& p, Zip::CompressionOptions o)
{
    Q_UNUSED(o);
	QDir d(QDir::cleanPath(p));
	if (!d.exists())
		return QString();

	if (!d.cdUp())
		return QString();

	return d.absolutePath();
}


/************************************************************************
 Public interface
*************************************************************************/

/*!
	Creates a new Zip file compressor.
*/
Zip::Zip() : d(new ZipPrivate)
{
}

/*!
	Closes any open archive and releases used resources.
*/
Zip::~Zip()
{
	closeArchive();
	delete d;
}

/*!
	Returns true if there is an open archive.
*/
bool Zip::isOpen() const
{
	return d->device;
}

/*!
	Sets the password to be used for the next files being added!
	Files added before calling this method will use the previously
	set password (if any).
	Closing the archive won't clear the password!
*/
void Zip::setPassword(const QString& pwd)
{
	d->password = pwd;
}

//! Convenience method, clears the current password.
void Zip::clearPassword()
{
	d->password.clear();
}

//! Returns the currently used password.
QString Zip::password() const
{
	return d->password;
}

/*!
	Attempts to create a new Zip archive. If \p overwrite is true and the file
	already exist it will be overwritten.
	Any open archive will be closed.
 */
Zip::ErrorCode Zip::createArchive(const QString& filename, bool overwrite)
{
    closeArchive();
    Q_ASSERT(!d->device && !d->file);

    if (filename.isEmpty())
        return Zip::FileNotFound;

	d->file = new QFile(filename);

	if (d->file->exists() && !overwrite) {
        delete d->file;
        d->file = 0;
		return Zip::FileExists;
	}

	if (!d->file->open(QIODevice::WriteOnly)) {
        delete d->file;
        d->file = 0;
		return Zip::OpenFailed;
	}

	const Zip::ErrorCode ec = createArchive(d->file);
	if (ec != Zip::Ok) {
		closeArchive();
	}

	return ec;
}

/*!
	Attempts to create a new Zip archive. If there is another open archive this will be closed.
	\warning The class takes ownership of the device!
 */
Zip::ErrorCode Zip::createArchive(QIODevice* device)
{
	if (!device) {
		qDebug() << "Invalid device.";
		return Zip::OpenFailed;
	}

	return d->createArchive(device);
}

/*!
	Returns the current archive comment.
*/
QString Zip::archiveComment() const
{
	return d->comment;
}

/*!
	Sets the comment for this archive. Note: createArchive() should have been
	called before.
*/
void Zip::setArchiveComment(const QString& comment)
{
	d->comment = comment;
}

/*!
    Convenience method, same as calling Zip::addDirectory(const QString&,const QString&,CompressionOptions,CompressionLevel)
    with the Zip::IgnorePaths flag as compression option and an empty \p root parameter.

    The result is that all files found in \p path (and in subdirectories) are
    added to the zip file without a directory entry.
*/
Zip::ErrorCode Zip::addDirectoryContents(const QString& path, CompressionLevel level)
{
    return addDirectory(path, QString(), IgnorePaths, level);
}

/*!
    Convenience method, same as calling Zip::addDirectory(const QString&,const QString&,CompressionOptions,CompressionLevel)
    with the Zip::IgnorePaths flag as compression option.

    The result is that all files found in \p path (and in subdirectories) are
    added to the zip file without a directory entry (or within a directory
    structure specified by \p root).
*/
Zip::ErrorCode Zip::addDirectoryContents(const QString& path, const QString& root, CompressionLevel level)
{
    return addDirectory(path, root, IgnorePaths, level);
}

/*!
	Convenience method, same as calling
	Zip::addDirectory(const QString&,const QString&,CompressionLevel)
    with an empty \p root parameter and Zip::RelativePaths flag as compression option.
 */
Zip::ErrorCode Zip::addDirectory(const QString& path, CompressionLevel level)
{
    return addDirectory(path, QString(), Zip::RelativePaths, level);
}

/*!
	Convenience method, same as calling Zip::addDirectory(const QString&,const QString&,CompressionOptions,CompressionLevel)
	with the Zip::RelativePaths flag as compression option.
 */
Zip::ErrorCode Zip::addDirectory(const QString& path, const QString& root, CompressionLevel level)
{
	return addDirectory(path, root, Zip::RelativePaths, level);
}

/*!
	Recursively adds files contained in \p dir to the archive, using \p root as name for the root folder.
	Stops adding files if some error occurs.

	The ExtractionOptions are checked in the order they are defined in the zip.h heaser file.
	This means that the last one overwrites the previous one (if some conflict occurs), i.e.
	Zip::IgnorePaths | Zip::AbsolutePaths would be interpreted as Zip::IgnorePaths.

	The \p root parameter is ignored with the Zip::IgnorePaths parameter and used as path prefix (a trailing /
	is always added as directory separator!) otherwise (even with Zip::AbsolutePaths set!).

    If \p addedFiles is not null it is set to the number of successfully added
    files.
*/
Zip::ErrorCode Zip::addDirectory(const QString& path, const QString& root,
    CompressionOptions options, CompressionLevel level, int* addedFiles)
{
    const int hierarchyLev = 0;
    return d->addDirectory(path, root, options, level, hierarchyLev, addedFiles);
}

/*!
    Convenience method, same as calling Zip::addFile(const QString&,const QString&,CompressionOptions,CompressionLevel)
    with an empty \p root parameter and Zip::RelativePaths as compression option.
 */
Zip::ErrorCode Zip::addFile(const QString& path, CompressionLevel level)
{
    return addFile(path, QString(), Zip::RelativePaths, level);
}

/*!
    Convenience method, same as calling Zip::addFile(const QString&,const QString&,CompressionOptions,CompressionLevel)
    with the Zip::RelativePaths flag as compression option.
 */
Zip::ErrorCode Zip::addFile(const QString& path, const QString& root,
    CompressionLevel level)
{
    return addFile(path, root, Zip::RelativePaths, level);
}

/*!
    Adds the file at \p path to the archive, using \p root as name for the root folder.
    If \p path points to a directory the behaviour is basically the same as
    addDirectory().

    The ExtractionOptions are checked in the order they are defined in the zip.h heaser file.
    This means that the last one overwrites the previous one (if some conflict occurs), i.e.
    Zip::IgnorePaths | Zip::AbsolutePaths would be interpreted as Zip::IgnorePaths.

    The \p root parameter is ignored with the Zip::IgnorePaths parameter and used as path prefix (a trailing /
    is always added as directory separator!) otherwise (even with Zip::AbsolutePaths set!).
*/
Zip::ErrorCode Zip::addFile(const QString& path, const QString& root,
    CompressionOptions options, CompressionLevel level)
{
    if (path.isEmpty())
        return Zip::Ok;
    return addFiles(QStringList() << path, root, options, level);
}

/*!
    Convenience method, same as calling Zip::addFiles(const QStringList&,const QString&,CompressionOptions,CompressionLevel)
    with an empty \p root parameter and Zip::RelativePaths as compression option.
 */
Zip::ErrorCode Zip::addFiles(const QStringList& paths, CompressionLevel level)
{
    return addFiles(paths, QString(), Zip::RelativePaths, level);
}

/*!
    Convenience method, same as calling Zip::addFiles(const QStringList&,const QString&,CompressionOptions,CompressionLevel)
    with the Zip::RelativePaths flag as compression option.
 */
Zip::ErrorCode Zip::addFiles(const QStringList& paths, const QString& root,
    CompressionLevel level)
{
    return addFiles(paths, root, Zip::RelativePaths, level);
}

/*!
    Adds the files or directories in \p paths to the archive, using \p root as
    name for the root folder.
    This is similar to calling addFile or addDirectory for all the entries in
    \p paths, except it is slightly faster.

    The ExtractionOptions are checked in the order they are defined in the zip.h heaser file.
    This means that the last one overwrites the previous one (if some conflict occurs), i.e.
    Zip::IgnorePaths | Zip::AbsolutePaths would be interpreted as Zip::IgnorePaths.

    The \p root parameter is ignored with the Zip::IgnorePaths parameter and used as path prefix (a trailing /
    is always added as directory separator!) otherwise (even with Zip::AbsolutePaths set!).

    If \p addedFiles is not null it is set to the number of successfully added
    files.
*/
Zip::ErrorCode Zip::addFiles(const QStringList& paths, const QString& root,
    CompressionOptions options, CompressionLevel level, int* addedFiles)
{
    return d->addFiles(paths, root, options, level, addedFiles);
}

/*!
	Closes the archive and writes any pending data.
*/
Zip::ErrorCode Zip::closeArchive()
{
	Zip::ErrorCode ec = d->closeArchive();
	d->reset();
	return ec;
}

/*!
	Returns a locale translated error string for a given error code.
*/
QString Zip::formatError(Zip::ErrorCode c) const
{
	switch (c)
	{
	case Ok: return QCoreApplication::translate("Zip", "ZIP operation completed successfully."); break;
	case ZlibInit: return QCoreApplication::translate("Zip", "Failed to initialize or load zlib library."); break;
	case ZlibError: return QCoreApplication::translate("Zip", "zlib library error."); break;
	case OpenFailed: return QCoreApplication::translate("Zip", "Unable to create or open file."); break;
	case NoOpenArchive: return QCoreApplication::translate("Zip", "No archive has been created yet."); break;
	case FileNotFound: return QCoreApplication::translate("Zip", "File or directory does not exist."); break;
	case ReadFailed: return QCoreApplication::translate("Zip", "File read error."); break;
	case WriteFailed: return QCoreApplication::translate("Zip", "File write error."); break;
	case SeekFailed: return QCoreApplication::translate("Zip", "File seek error."); break;
	default: ;
	}

	return QCoreApplication::translate("Zip", "Unknown error.");
}

OSDAB_END_NAMESPACE
