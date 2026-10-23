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
    : QObject(parent), logToConsole(_logToConsole), writeLog(_writeLog),
      messageFilterActive(!_logFilters.trimmed().isEmpty()),
      logFilters(_logFilters.split(QStringLiteral(","), Qt::SkipEmptyParts)), flushRunning(false)
{
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

    // The console is an independent sink: writelog and logfilters only describe the logfile
    // (see servatrice.ini.example), so a disabled or unopenable logfile must not swallow
    // startup errors such as a refused database connection.
    if (logToConsole) {
        std::cout << (line + QLatin1Char('\n')).toStdString() << std::flush;
    }

    configMutex.lock();
    const bool shouldWriteFile = writeLog;
    const bool filterMessages = messageFilterActive;
    const QStringList filters = logFilters;
    configMutex.unlock();

    if (!logFile || !shouldWriteFile) {
        return;
    }

    // filter out all log entries based on values in configuration file
    if (filterMessages) {
        bool shouldWeSkipLine = true;
        for (const QString &logFilter : filters) {
            if (message.contains(logFilter, Qt::CaseInsensitive)) {
                shouldWeSkipLine = false;
                break;
            }
        }
        if (shouldWeSkipLine) {
            return;
        }
    }

    bufferMutex.lock();
    buffer.append(line);
    bufferMutex.unlock();
    emit sigFlushBuffer();
}

void ServerLogger::flushBuffer()
{
    if (!logFile) {
        return;
    }

    if (flushRunning) {
        return;
    }

    flushRunning = true;
    QTextStream stream(logFile);
    forever
    {
        bufferMutex.lock();
        if (buffer.isEmpty()) {
            bufferMutex.unlock();
            flushRunning = false;
            return;
        }
        QString message = buffer.takeFirst();
        bufferMutex.unlock();

        stream << message << "\n";
        stream.flush();
    }
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
