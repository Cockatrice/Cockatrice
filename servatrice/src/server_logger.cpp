#include "server_logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QForeach>
#include <QIODevice>
#include <QTextStream>
#include <QThread>
#include <QtGlobal>
#include <iostream>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <string>

ServerLogger::ServerLogger(bool _logToConsole, bool _writeLog, const QString &_logFilters, QObject *parent)
    : QObject(parent), logToConsole(_logToConsole), flushRunning(false)
{
    reloadConfig(_writeLog, _logFilters);
    connect(this, SIGNAL(sigFlushBuffer()), this, SLOT(flushBuffer()), Qt::QueuedConnection);
}

ServerLogger::~ServerLogger()
{
    flushBuffer();
    // logFile is a child of this object and is closed and deleted by ~QObject.
    // This does not work with the destroyed() signal as this destructor is called after the main event loop is done.
    thread()->quit();
}

void ServerLogger::startLog(const QString &logFileName)
{
    delete logFile;
    logFile = nullptr;

    if (!logFileName.isEmpty()) {
        QFileInfo fi(logFileName);
        QDir fileDir(fi.path());
        if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
            std::cerr << "ERROR: logfile folder doesn't exist and i can't create it." << std::endl;
            return;
        }

        logFile = new QFile(logFileName, this);
        if (!logFile->open(QIODevice::Append)) {
            std::cerr << "ERROR: can't open() logfile." << std::endl;
            delete logFile;
            logFile = nullptr;
            return;
        }
    }
}

void ServerLogger::reloadConfig(bool _writeLog, const QString &_logFilters)
{
    const bool filterActive = !_logFilters.trimmed().isEmpty();
    const QStringList filters = _logFilters.split(QStringLiteral(","), Qt::SkipEmptyParts);

    configMutex.lock();
    writeLog = _writeLog;
    messageFilterActive = filterActive;
    logFilters = filters;
    configMutex.unlock();
}

void ServerLogger::logMessage(const QString &message, void *caller)
{
    QString callerString;
    if (caller) {
        callerString = QString::number(reinterpret_cast<qulonglong>(caller), 16) + " ";
    }

    const QString line = QDateTime::currentDateTime().toString() + " " + callerString + message;
    bool queued = false;

    // The console is an independent sink: writelog and logfilters only describe the logfile
    // (see servatrice.ini.example), so a disabled or unopenable logfile must not swallow
    // startup errors such as a refused database connection. Lines are buffered for the logger
    // thread instead of written here, so a stalled stdout reader cannot stall game threads.
    if (logToConsole) {
        bufferMutex.lock();
        consoleBuffer.append(line);
        bufferMutex.unlock();
        queued = true;
    }

    configMutex.lock();
    const bool shouldWriteFile = writeLog;
    const bool filterMessages = messageFilterActive;
    const QStringList filters = logFilters;
    configMutex.unlock();

    if (logFile && shouldWriteFile) {
        // filter out all log entries based on values in configuration file
        bool shouldWriteLine = !filterMessages;
        if (filterMessages) {
            for (const QString &logFilter : filters) {
                if (message.contains(logFilter, Qt::CaseInsensitive)) {
                    shouldWriteLine = true;
                    break;
                }
            }
        }

        if (shouldWriteLine) {
            bufferMutex.lock();
            buffer.append(line);
            bufferMutex.unlock();
            queued = true;
        }
    }

    if (queued) {
        emit sigFlushBuffer();
    }
}

void ServerLogger::flushBuffer()
{
    if (flushRunning) {
        return;
    }

    flushRunning = true;

    bufferMutex.lock();
    QStringList consoleLines;
    consoleLines.swap(consoleBuffer);
    bufferMutex.unlock();

    for (const QString &line : consoleLines) {
        std::cout << line.toStdString() << '\n';
    }
    if (!consoleLines.isEmpty()) {
        std::cout.flush();
    }

    if (logFile) {
        QTextStream stream(logFile);
        forever
        {
            bufferMutex.lock();
            if (buffer.isEmpty()) {
                bufferMutex.unlock();
                break;
            }
            QString message = buffer.takeFirst();
            bufferMutex.unlock();

            stream << message << "\n";
            stream.flush();
        }
    } else {
        // No logfile to write to; do not let the buffer grow without bound.
        bufferMutex.lock();
        buffer.clear();
        bufferMutex.unlock();
    }

    flushRunning = false;
}

void ServerLogger::rotateLogs()
{
    if (!logFile) {
        return;
    }

    flushBuffer();

    logFile->close();
    if (!logFile->open(QIODevice::Append)) {
        std::cerr << "ERROR: Failed to open log file for writing!" << std::endl;
    }
}
