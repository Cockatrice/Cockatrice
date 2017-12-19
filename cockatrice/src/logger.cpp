#include "logger.h"
#include "version_string.h"
#include <QDateTime>
#include <iostream>

#define LOGGER_MAX_ENTRIES 128
#define LOGGER_FILENAME "qdebug.txt"

Logger::Logger() : logToFileEnabled(false)
{
    logBuffer.append(getClientVersion());
    logBuffer.append(getBuildArchitecture());
    std::cerr << getClientVersion().toStdString() << std::endl;
    std::cerr << getBuildArchitecture().toStdString() << std::endl;
}

Logger::~Logger()
{
    closeLogfileSession();
    logBuffer.clear();
}

void Logger::logToFile(bool enabled)
{
    if (enabled)
    {
        openLogfileSession();
    }
    else
    {
        closeLogfileSession();
    }
}

QString Logger::getClientVersion()
{
    return "Client Version: " + QString::fromStdString(VERSION_STRING);
}

void Logger::openLogfileSession()
{
    if (logToFileEnabled)
    {
        return;
    }

    fileHandle.setFileName(LOGGER_FILENAME);
    fileHandle.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    fileStream.setDevice(&fileHandle);
    fileStream << "Log session started at " << QDateTime::currentDateTime().toString() << endl;
    fileStream << getClientVersion() << endl;
    fileStream << getBuildArchitecture() << endl;
    logToFileEnabled = true;
}

void Logger::closeLogfileSession()
{
    if (!logToFileEnabled)
        return;

    logToFileEnabled = false;
    fileStream << "Log session closed at " << QDateTime::currentDateTime().toString() << endl;
    fileHandle.close();
}

void Logger::log(QtMsgType /* type */, const QMessageLogContext & /* ctx */, const QString message)
{
    QMetaObject::invokeMethod(this, "internalLog", Qt::QueuedConnection, Q_ARG(const QString &, message));
}

void Logger::internalLog(const QString message)
{
    QMutexLocker locker(&mutex);

    logBuffer.append(message);
    if (logBuffer.size() > LOGGER_MAX_ENTRIES)
    {
        logBuffer.removeAt(1);
    }

    emit logEntryAdded(message);
    std::cerr << message.toStdString() << std::endl; // Print to stdout

    if (logToFileEnabled)
        fileStream << message << endl; // Print to fileStream
}

QString Logger::getBuildArchitecture()
{
#if   defined(Q_OS_MACOS)
  QString systemOS = "macOS";
#elif defined(Q_OS_LINUX)
  QString systemOS = "Linux";
#elif defined(Q_OS_WIN)
  QString systemOS = "Windows";
#elif defined(Q_OS_ANDROID)
  QString systemOS = "Android";
#else
  QString systemOS = "unknown";
#endif
#if   defined(Q_PROCESSOR_X86_32)
  QString arch = "32-bit";
#elif defined(Q_PROCESSOR_X86_64)
  QString arch = "64-bit";
#elif defined(Q_PROCESSOR_ARM)
  QString arch = "ARM";
#else
  QString arch = "unknown";
#endif
    return "Client Operating System: " + systemOS + "\n" +
           "Build Architecture: " + arch + "\n" +
           "Qt Version: " + QT_VERSION_STR;
}
