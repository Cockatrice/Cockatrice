#include "logger.h"

#include <QDateTime>

#define LOGGER_MAX_ENTRIES 128
#define LOGGER_FILENAME "qdebug.txt"

Logger::Logger()
: logToFileEnabled(false)
{
}

Logger::~Logger()
{
    closeLogfileSession();
    logBuffer.clear();
}

void Logger::logToFile(bool enabled)
{
    if(enabled)
        openLogfileSession();
    else
        closeLogfileSession();
}

void Logger::openLogfileSession()
{
    if(logToFileEnabled)
        return;

    fileHandle.setFileName(LOGGER_FILENAME);
    fileHandle.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    fileStream.setDevice(&fileHandle);
    fileStream << "Log session started at " << QDateTime::currentDateTime().toString() << endl;
    logToFileEnabled = true;
}

void Logger::closeLogfileSession()
{
    if(!logToFileEnabled)
        return;

    logToFileEnabled = false;
    fileStream << "Log session closed at " << QDateTime::currentDateTime().toString() << endl;
    fileHandle.close();
}


void Logger::log(QtMsgType /* type */, const QMessageLogContext & /* ctx */, const QString &message)
{
    logBuffer.append(message);
    if(logBuffer.size() > LOGGER_MAX_ENTRIES)
        logBuffer.removeFirst();

    emit logEntryAdded(message);

    if(logToFileEnabled)
    {
        fileStream << message << endl;
    }
}