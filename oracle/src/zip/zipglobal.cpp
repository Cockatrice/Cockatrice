/****************************************************************************
** Filename: zipglobal.cpp
** Last updated [dd/mm/yyyy]: 06/02/2011
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

#include "zipglobal.h"

#if defined(Q_OS_WIN) || defined(Q_OS_WINCE) || defined(Q_OS_LINUX) || defined (Q_OS_MACX)
#define OSDAB_ZIP_HAS_UTC
#include <ctime>
#else
#undef OSDAB_ZIP_HAS_UTC
#endif

#if defined(Q_OS_WIN)
#include <QtCore/qt_windows.h>
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACX)
#include <utime.h>
#endif

OSDAB_BEGIN_NAMESPACE(Zip)

/*! Returns the current UTC offset in seconds unless OSDAB_ZIP_NO_UTC is defined
    and method is implemented for the current platform and 0 otherwise.
*/
int OSDAB_ZIP_MANGLE(currentUtcOffset)()
{
#if !(!defined OSDAB_ZIP_NO_UTC && defined OSDAB_ZIP_HAS_UTC)
    return 0;
#else
    time_t curr_time_t;
    time(&curr_time_t);

#if defined Q_OS_WIN
    struct tm _tm_struct;
    struct tm* tm_struct = &_tm_struct;
#else
    struct tm* tm_struct = 0;
#endif

#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tzset();
    tm res;
    tm_struct = gmtime_r(&curr_time_t, &res);
#elif defined Q_OS_WIN && !defined Q_CC_MINGW
    if (gmtime_s(tm_struct, &curr_time_t))
        return 0;
#else
    tm_struct = gmtime(&curr_time_t);
#endif

    if (!tm_struct)
        return 0;

    const time_t global_time_t = mktime(tm_struct);

#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tm_struct = localtime_r(&curr_time_t, &res);
#elif defined Q_OS_WIN && !defined Q_CC_MINGW
    if (localtime_s(tm_struct, &curr_time_t))
        return 0;
#else
    tm_struct = localtime(&curr_time_t);
#endif

    if (!tm_struct)
        return 0;

    const time_t local_time_t = mktime(tm_struct);

    const int utcOffset = - qRound(difftime(global_time_t, local_time_t));
    return tm_struct->tm_isdst > 0 ? utcOffset + 3600 : utcOffset;
#endif // No UTC

    return 0;
}

QDateTime OSDAB_ZIP_MANGLE(fromFileTimestamp)(const QDateTime& dateTime)
{
#if !defined OSDAB_ZIP_NO_UTC && defined OSDAB_ZIP_HAS_UTC
    const int utc = OSDAB_ZIP_MANGLE(currentUtcOffset)();
    return dateTime.toUTC().addSecs(utc);
#else
    return dateTime;
#endif // OSDAB_ZIP_NO_UTC
}

bool OSDAB_ZIP_MANGLE(setFileTimestamp)(const QString& fileName, const QDateTime& dateTime)
{
    if (fileName.isEmpty())
        return true;

#ifdef Q_OS_WIN
    HANDLE hFile = CreateFile(fileName.toStdWString().c_str(),
        GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    SYSTEMTIME st;
    FILETIME ft, ftLastMod;
    const QDate date = dateTime.date();
    const QTime time = dateTime.time();
    st.wYear = date.year();
    st.wMonth = date.month();
    st.wDay = date.day();
    st.wHour = time.hour();
    st.wMinute = time.minute();
    st.wSecond = time.second();
    st.wMilliseconds = time.msec();

    SystemTimeToFileTime(&st, &ft);
    LocalFileTimeToFileTime(&ft, &ftLastMod);

    const bool success = SetFileTime(hFile, NULL, NULL, &ftLastMod);
    CloseHandle(hFile);
    return success;

#elif defined(Q_OS_LINUX) || defined(Q_OS_MACX)

    struct utimbuf t_buffer;
    t_buffer.actime = t_buffer.modtime = dateTime.toTime_t();
    return utime(fileName.toLocal8Bit().constData(), &t_buffer) == 0;
#endif

    return true;
}
OSDAB_END_NAMESPACE
