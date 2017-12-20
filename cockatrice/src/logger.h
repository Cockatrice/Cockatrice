#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QFile>
#include <QVector>
#include <QString>
#include <QMutex>

#if   defined(Q_PROCESSOR_X86_32)
  #define BUILD_ARCHITECTURE "32-bit"
#elif defined(Q_PROCESSOR_X86_64)
  #define BUILD_ARCHITECTURE "64-bit"
#elif defined(Q_PROCESSOR_ARM)
  #define BUILD_ARCHITECTURE "ARM"
#else
  #define BUILD_ARCHITECTURE "unknown"
#endif

class Logger : public QObject {
Q_OBJECT
public:
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }
private:
    Logger();
    ~Logger();
    // Singleton - Don't implement copy constructor and assign operator
    Logger(Logger const&);
    void operator=(Logger const&); 

    bool logToFileEnabled;
    QTextStream fileStream;
    QFile fileHandle;
    QList<QString> logBuffer;
    QMutex mutex;
public:
	void logToFile(bool enabled);
    void log(QtMsgType type, const QMessageLogContext &ctx, const QString message);
    QString getClientVersion();
    QString getClientOperatingSystem();
    QString printBuildArchitecture();
    QList<QString> getLogBuffer() { return logBuffer; }
protected:
    void openLogfileSession();
    void closeLogfileSession();
protected slots:
    void internalLog(const QString message);
signals:
    void logEntryAdded(QString message);
};

#endif
