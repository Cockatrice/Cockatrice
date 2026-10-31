#include "logger.h"

#include "version_string.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFlags>
#include <QIODevice>
#include <QLocale>
#include <QMetaObject>
#include <QMetaType>
#include <QMutexLocker>
#include <QSysInfo>
#include <iostream>
#include <qconfig.h>
#include <qnamespace.h>

#define LOGGER_MAX_ENTRIES 128
#define LOGGER_FILENAME "qdebug.txt"
#define LOGGER_SESSION_MARKER "Log session started at"
#define LOGGER_PERSISTENT_MAX_BYTES (512 * 1024)
#define LOGGER_PERSISTENT_KEEP_LINES 2000

Logger::Logger() : logToFileEnabled(false), persistentLogEnabled(false)
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
    closePersistentLogSession();
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
    fileStream << "Log session started at " << QDateTime::currentDateTime().toString() << Qt::endl;
    fileStream << getClientVersion() << Qt::endl;
    fileStream << getSystemArchitecture() << Qt::endl;
    fileStream << getClientInstallInfo() << Qt::endl;
    logToFileEnabled = true;
}

void Logger::closeLogfileSession()
{
    if (!logToFileEnabled) {
        return;
    }

    logToFileEnabled = false;
    fileStream << "Log session closed at " << QDateTime::currentDateTime().toString() << Qt::endl;
    fileHandle.close();
}

void Logger::setPersistentLogPath(const QString &path)
{
    QMutexLocker locker(&mutex);
    if (path.isEmpty() || (persistentLogEnabled && persistentLogPath == path)) {
        return;
    }

    QDir().mkpath(QFileInfo(path).absolutePath());
    persistentLogPath = path;
    openPersistentLog(path);
}

void Logger::prunePersistentLog(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists() || info.size() <= LOGGER_PERSISTENT_MAX_BYTES) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QList<QString> allLines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    file.close();

    const int start =
        allLines.size() > LOGGER_PERSISTENT_KEEP_LINES ? allLines.size() - LOGGER_PERSISTENT_KEEP_LINES : 0;
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    for (int i = start; i < allLines.size(); ++i) {
        out << allLines.at(i) << Qt::endl;
    }
    file.close();
}

void Logger::openPersistentLog(const QString &path)
{
    prunePersistentLog(path);

    persistentFileHandle.setFileName(path);
    if (!persistentFileHandle.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Logger failed to open persistent log" << path << "for writing";
        return;
    }
    persistentFileStream.setDevice(&persistentFileHandle);
    persistentFileStream << LOGGER_SESSION_MARKER << " " << QDateTime::currentDateTime().toString() << Qt::endl;
    persistentFileStream << getClientVersion() << Qt::endl;
    persistentFileStream << getSystemArchitecture() << Qt::endl;
    persistentFileStream << getClientInstallInfo() << Qt::endl;
    persistentFileStream.flush();
    persistentLogEnabled = true;
}

void Logger::closePersistentLogSession()
{
    if (!persistentLogEnabled) {
        return;
    }

    persistentLogEnabled = false;
    persistentFileStream << "Log session closed at " << QDateTime::currentDateTime().toString() << Qt::endl;
    persistentFileStream.flush();
    persistentFileHandle.close();
}

QList<QString> Logger::getPreviousSessionTail(int maxLines) const
{
    QString path;
    {
        QMutexLocker locker(&mutex);
        path = persistentLogPath;
    }
    if (path.isEmpty()) {
        return {};
    }
    return extractPreviousSessionTail(path, maxLines);
}

QList<QString> Logger::extractPreviousSessionTail(const QString &filePath, int maxLines)
{
    QList<QString> result;
    if (maxLines <= 0) {
        return result;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }
    QList<QString> allLines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    file.close();

    QList<int> markerIndexes;
    for (int i = 0; i < allLines.size(); ++i) {
        if (allLines.at(i).contains(QLatin1String(LOGGER_SESSION_MARKER))) {
            markerIndexes.append(i);
        }
    }

    QList<QString> tail;
    if (markerIndexes.isEmpty()) {
        tail = allLines;
    } else if (markerIndexes.size() == 1) {
        // Only the current session has been recorded; there is no previous session yet.
        return result;
    } else {
        const int previousStart = markerIndexes.at(markerIndexes.size() - 2);
        const int currentStart = markerIndexes.last();
        for (int i = previousStart; i < currentStart; ++i) {
            tail.append(allLines.at(i));
        }
    }

    const int start = tail.size() > maxLines ? tail.size() - maxLines : 0;
    for (int i = start; i < tail.size(); ++i) {
        result.append(tail.at(i));
    }
    return result;
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
        fileStream << message << Qt::endl; // Print to fileStream
    }

    if (persistentLogEnabled) {
        persistentFileStream << message << Qt::endl;
        persistentFileStream.flush();
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
