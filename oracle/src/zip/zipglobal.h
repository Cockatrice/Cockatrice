/****************************************************************************
** Filename: zipglobal.h
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

#ifndef OSDAB_ZIPGLOBAL__H
#define OSDAB_ZIPGLOBAL__H

#include <QtCore/QDateTime>
#include <QtCore/QtGlobal>

/* If you want to build the OSDaB Zip code as
   a library, define OSDAB_ZIP_LIB in the library's .pro file and
   in the libraries using it OR remove the #ifndef OSDAB_ZIP_LIB
   define below and leave the #else body. Also remember to define
   OSDAB_ZIP_BUILD_LIB in the library's project).
*/

#ifndef OSDAB_ZIP_LIB
# define OSDAB_ZIP_EXPORT
#else
# if defined(OSDAB_ZIP_BUILD_LIB)
#   define OSDAB_ZIP_EXPORT Q_DECL_EXPORT
# else
#   define OSDAB_ZIP_EXPORT Q_DECL_IMPORT
# endif
#endif

#ifdef OSDAB_NAMESPACE
#define OSDAB_BEGIN_NAMESPACE(ModuleName) namespace Osdab { namespace ModuleName {
#else
#define OSDAB_BEGIN_NAMESPACE(ModuleName)
#endif

#ifdef OSDAB_NAMESPACE
#define OSDAB_END_NAMESPACE } }
#else
#define OSDAB_END_NAMESPACE
#endif

#ifndef OSDAB_NAMESPACE
#define OSDAB_ZIP_MANGLE(x) zip_##x
#else
#define OSDAB_ZIP_MANGLE(x) x
#endif

OSDAB_BEGIN_NAMESPACE(Zip)

OSDAB_ZIP_EXPORT int OSDAB_ZIP_MANGLE(currentUtcOffset)();
OSDAB_ZIP_EXPORT QDateTime OSDAB_ZIP_MANGLE(fromFileTimestamp)(const QDateTime& dateTime);
OSDAB_ZIP_EXPORT bool OSDAB_ZIP_MANGLE(setFileTimestamp)(const QString& fileName, const QDateTime& dateTime);

OSDAB_END_NAMESPACE

#endif // OSDAB_ZIPGLOBAL__H
