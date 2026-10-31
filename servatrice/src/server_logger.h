#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qtmetamacros.h>

class QFile;

class ServerLogger : public QObject
{
    Q_OBJECT
public:
    ServerLogger(bool _logToConsole, bool _writeLog, const QString &_logFilters, QObject *parent = nullptr);
    ~ServerLogger();

    /**
     * @brief Whether messages passed to logMessage() would be written anywhere.
     *
     * Callers use this to avoid formatting expensive debug payloads that
     * logMessage() would discard.
     */
    [[nodiscard]] bool isLoggingEnabled() const;

public slots:
    void startLog(const QString &logFileName);
    void reloadConfig(bool _writeLog, const QString &_logFilters);
    void logMessage(const QString &message, void *caller = 0);
    void rotateLogs();
private slots:
    void flushBuffer();
signals:
    void sigFlushBuffer();

private:
    bool logToConsole;
    bool writeLog = true;
    bool messageFilterActive = false;
    QStringList logFilters;
    // Guards writeLog/messageFilterActive/logFilters: SIGHUP rewrites them while other threads log.
    QMutex configMutex;
    QFile *logFile = nullptr;
    bool flushRunning;
    QStringList buffer;
    // Console lines wait here for the logger thread, so game threads never block on stdout.
    QStringList consoleBuffer;
    // Guards buffer and consoleBuffer.
    QMutex bufferMutex;
};

#endif
