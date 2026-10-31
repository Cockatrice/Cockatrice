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
    ServerLogger(bool _logToConsole, QObject *parent = 0);
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
    void logMessage(const QString &message, void *caller = 0);
    void rotateLogs();
private slots:
    void flushBuffer();
signals:
    void sigFlushBuffer();

private:
    bool logToConsole;
    static QFile *logFile;
    bool flushRunning;
    QStringList buffer;
    QMutex bufferMutex;
};

#endif
