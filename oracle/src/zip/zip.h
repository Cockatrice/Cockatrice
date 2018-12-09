/****************************************************************************
** Filename: zip.h
** Last updated [dd/mm/yyyy]: 27/03/2011
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

#ifndef OSDAB_ZIP__H
#define OSDAB_ZIP__H

#include "zipglobal.h"

#include <QtCore/QMap>
#include <QtCore/QtGlobal>

#include <zlib/zlib.h>

class QIODevice;
class QFile;
class QDir;
class QStringList;
class QString;

OSDAB_BEGIN_NAMESPACE(Zip)

class ZipPrivate;

class OSDAB_ZIP_EXPORT Zip
{
public:
	enum ErrorCode
	{
		Ok,
		ZlibInit,
		ZlibError,
		FileExists,
		OpenFailed,
		NoOpenArchive,
		FileNotFound,
		ReadFailed,
		WriteFailed,
        SeekFailed,
        InternalError
	};

	enum CompressionLevel
	{
		Store,
		Deflate1 = 1, Deflate2, Deflate3, Deflate4,
		Deflate5, Deflate6, Deflate7, Deflate8, Deflate9,
		AutoCPU, AutoMIME, AutoFull
	};

	enum CompressionOption
	{
        /*! Does not preserve absolute paths in the zip file when adding a
            file or directory (default) */
		RelativePaths = 0x0001,
        /*! Preserve absolute paths */
		AbsolutePaths = 0x0002,
        /*! Do not store paths. All the files are put in the (evtl. user defined)
            root of the zip file */
        IgnorePaths = 0x0004,
        /*! Works only with addDirectory(). Adds the directory's contents,
            including subdirectories, but does not add an entry for the root
            directory itself. */
        IgnoreRoot = 0x0008,
        /*! Used only when compressing a directory or multiple files.
            If set invalid or unreadable files are simply skipped.
        */
        SkipBadFiles = 0x0020,
        /*! Makes sure a file is never added twice to the same zip archive.
            This check is only necessary in certain usage scenarios and given
            that it slows down processing you need to enable it explicitly with
            this flag.
        */
        CheckForDuplicates = 0x0040
	};
	Q_DECLARE_FLAGS(CompressionOptions, CompressionOption)

	Zip();
	virtual ~Zip();

	bool isOpen() const;

	void setPassword(const QString& pwd);
	void clearPassword();
	QString password() const;

	ErrorCode createArchive(const QString& file, bool overwrite = true);
	ErrorCode createArchive(QIODevice* device);

	QString archiveComment() const;
	void setArchiveComment(const QString& comment);

    ErrorCode addDirectoryContents(const QString& path,
        CompressionLevel level = AutoFull);
    ErrorCode addDirectoryContents(const QString& path, const QString& root,
        CompressionLevel level = AutoFull);

    ErrorCode addDirectory(const QString& path,
        CompressionLevel level = AutoFull);
    ErrorCode addDirectory(const QString& path, const QString& root,
        CompressionLevel level = AutoFull);
    ErrorCode addDirectory(const QString& path, const QString& root,
        CompressionOptions options, CompressionLevel level = AutoFull,
        int* addedFiles = 0);

    ErrorCode addFile(const QString& path,
        CompressionLevel level = AutoFull);
    ErrorCode addFile(const QString& path, const QString& root,
        CompressionLevel level = AutoFull);
    ErrorCode addFile(const QString& path, const QString& root,
        CompressionOptions options,
        CompressionLevel level = AutoFull);

    ErrorCode addFiles(const QStringList& paths,
        CompressionLevel level = AutoFull);
    ErrorCode addFiles(const QStringList& paths, const QString& root,
        CompressionLevel level = AutoFull);
    ErrorCode addFiles(const QStringList& paths, const QString& root,
        CompressionOptions options,
        CompressionLevel level = AutoFull,
        int* addedFiles = 0);

	ErrorCode closeArchive();

	QString formatError(ErrorCode c) const;

private:
	ZipPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Zip::CompressionOptions)

OSDAB_END_NAMESPACE

#endif // OSDAB_ZIP__H
