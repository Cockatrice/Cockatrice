#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include <QMutex>
#include <QObject>
#include <QStringList>
#include <QThread>
#include <QWaitCondition>

class QFile;
class Server_ProtocolHandler;

class ServerLogger : public QObject
{
    Q_OBJECT
public:
    ServerLogger(bool _logToConsole, QObject *parent = 0);
    ~ServerLogger();
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
