#include "system_memory_querier.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#endif

#ifdef Q_OS_MACOS
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

qulonglong SystemMemoryQuerier::totalMemoryBytes()
{
#if defined(Q_OS_WIN)

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex)) {
        return statex.ullTotalPhys;
    }
    return 0;

#elif defined(Q_OS_LINUX)

    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("MemTotal:")) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
#else
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
#endif
            if (parts.size() >= 2) {
                return parts[1].toULongLong() * 1024; // kB → bytes
            }
        }
    }
    return 0;

#elif defined(Q_OS_MACOS)

    int mib[2] = {CTL_HW, HW_MEMSIZE};
    qulonglong memsize = 0;
    size_t len = sizeof(memsize);

    if (sysctl(mib, 2, &memsize, &len, nullptr, 0) == 0) {
        return memsize;
    }

    return 0;

#else
    return 0;
#endif
}
