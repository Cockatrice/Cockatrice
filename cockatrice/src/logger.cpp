#include "logger.h"
#include "version_string.h"
#include <QDateTime>
#include <iostream>

#define LOGGER_MAX_ENTRIES 128
#define LOGGER_FILENAME "qdebug.txt"

#if QT_VERSION >= 0x050400
#include <QSysInfo>
#endif

Logger::Logger() : logToFileEnabled(false)
{
    logBuffer.append(getClientVersion());
    logBuffer.append(getSystemArchitecture());
    std::cerr << getClientVersion().toStdString() << std::endl;
    std::cerr << getSystemArchitecture().toStdString() << std::endl;
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
    fileHandle.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    fileStream.setDevice(&fileHandle);
    fileStream << "Log session started at " << QDateTime::currentDateTime().toString() << endl;
    fileStream << getClientVersion() << endl;
    fileStream << getSystemArchitecture() << endl;
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
    if (logBuffer.size() > LOGGER_MAX_ENTRIES) {
        logBuffer.removeAt(1);
    }

    emit logEntryAdded(message);
    std::cerr << message.toStdString() << std::endl; // Print to stdout

    if (logToFileEnabled)
        fileStream << message << endl; // Print to fileStream
}

QString Logger::getSystemArchitecture()
{
    QString result;

    if (!getClientOperatingSystem().isEmpty()) {
        result.append("Client Operating System: " + getClientOperatingSystem() + "\n");
    }

    result.append("Build Architecture: " + QString::fromStdString(BUILD_ARCHITECTURE) + "\n");
    result.append("Qt Version: " + QT_VERSION_STR);

    return result;
}

QString Logger::getClientOperatingSystem()
{
#if QT_VERSION >= 0x050400
    return QSysInfo::prettyProductName();
#endif

    return {};
}
