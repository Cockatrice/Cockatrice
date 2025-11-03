#include "logger.h"

#include "version_string.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QLocale>
#include <QSysInfo>
#include <iostream>

#define LOGGER_MAX_ENTRIES 128
#define LOGGER_FILENAME "qdebug.txt"

Logger::Logger() : logToFileEnabled(false)
{
    logBuffer.append(getClientVersion());
    logBuffer.append(getSystemArchitecture());
    logBuffer.append(getSystemLocale());
    logBuffer.append(getClientInstallInfo());
    logBuffer.append(QString("-").repeated(75));
    std::cerr << getClientVersion().toStdString() << std::endl;
    std::cerr << getSystemArchitecture().toStdString() << std::endl;
    std::cerr << getSystemLocale().toStdString() << std::endl;
    std::cerr << getClientInstallInfo().toStdString() << std::endl;
}

Logger::~Logger()
{
    closeLogfileSession();
    logBuffer.clear();
}

void Logger::logToFile(bool enabled)
{
    if (enabled) {
        openLogfileSession();
    } else {
        closeLogfileSession();
    }
}

QString Logger::getClientVersion()
{
    return "Client Version: " + QString::fromStdString(VERSION_STRING);
}

void Logger::openLogfileSession()
{
    if (logToFileEnabled) {
        return;
    }

    fileHandle.setFileName(LOGGER_FILENAME);
    if (!fileHandle.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Logger failed to open" << LOGGER_FILENAME << "for writing";
        return;
    }
    fileStream.setDevice(&fileHandle);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    fileStream << "Log session started at " << QDateTime::currentDateTime().toString() << Qt::endl;
    fileStream << getClientVersion() << Qt::endl;
    fileStream << getSystemArchitecture() << Qt::endl;
    fileStream << getClientInstallInfo() << Qt::endl;
#else
    fileStream << "Log session started at " << QDateTime::currentDateTime().toString() << endl;
    fileStream << getClientVersion() << endl;
    fileStream << getSystemArchitecture() << endl;
    fileStream << getClientInstallInfo() << endl;
#endif
    logToFileEnabled = true;
}

void Logger::closeLogfileSession()
{
    if (!logToFileEnabled)
        return;

    logToFileEnabled = false;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    fileStream << "Log session closed at " << QDateTime::currentDateTime().toString() << Qt::endl;
#else
    fileStream << "Log session closed at " << QDateTime::currentDateTime().toString() << endl;
#endif
    fileHandle.close();
}

void Logger::log(QtMsgType /* type */, const QMessageLogContext & /* ctx */, const QString &message)
{
    QMetaObject::invokeMethod(this, "internalLog", Qt::QueuedConnection, Q_ARG(const QString &, message));
}

void Logger::internalLog(const QString &message)
{
    QMutexLocker locker(&mutex);

    logBuffer.append(message);
    if (logBuffer.size() > LOGGER_MAX_ENTRIES) {
        logBuffer.removeAt(1);
    }

    emit logEntryAdded(message);
    std::cerr << message.toStdString() << std::endl; // Print to stdout

    if (logToFileEnabled) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        fileStream << message << Qt::endl; // Print to fileStream
#else
        fileStream << message << endl; // Print to fileStream
#endif
    }
}

QString Logger::getSystemArchitecture()
{
    QString result;

    if (!getClientOperatingSystem().isEmpty()) {
        // We don't want translatable strings in the 'Debug Log' for easier troubleshooting
        result.append(QString("Client Operating System: ") + getClientOperatingSystem() + "\n");
    }

    result.append(QString("Build Architecture: ") + QString::fromStdString(BUILD_ARCHITECTURE) + "\n");
    result.append(QString("Qt Version: ") + QT_VERSION_STR);

    return result;
}

QString Logger::getClientOperatingSystem()
{
    return QSysInfo::prettyProductName();
}

QString Logger::getSystemLocale()
{
    QString result(QString("System Locale: ") + QLocale().name());
    return result;
}

QString Logger::getClientInstallInfo()
{
    // don't rely on settingsCache->getIsPortableBuild() since the logger is initialized earlier
    bool isPortable = QFile::exists(qApp->applicationDirPath() + "/portable.dat");
    QString result(QString("Install Mode: ") + (isPortable ? "Portable" : "Standard"));
    return result;
}
