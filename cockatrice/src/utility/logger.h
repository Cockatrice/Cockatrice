#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>
#include <QVector>

#if defined(Q_PROCESSOR_X86_32)
#define BUILD_ARCHITECTURE "32-bit"
#elif defined(Q_PROCESSOR_X86_64)
#define BUILD_ARCHITECTURE "64-bit"
#elif defined(Q_PROCESSOR_ARM)
#define BUILD_ARCHITECTURE "ARM"
#else
#define BUILD_ARCHITECTURE "unknown"
#endif

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    void logToFile(bool enabled);
    void log(QtMsgType type, const QMessageLogContext &ctx, QString message);
    QString getClientVersion();
    QString getClientOperatingSystem();
    QString getSystemArchitecture();
    QString getSystemLocale();
    QString getClientInstallInfo();
    QList<QString> getLogBuffer()
    {
        return logBuffer;
    }

private:
    Logger();
    ~Logger() override;
    // Singleton - Don't implement copy constructor and assign operator
    Logger(Logger const &);
    void operator=(Logger const &);

    bool logToFileEnabled;
    QTextStream fileStream;
    QFile fileHandle;
    QList<QString> logBuffer;
    QMutex mutex;

protected:
    void openLogfileSession();
    void closeLogfileSession();

protected slots:
    void internalLog(QString message);

signals:
    void logEntryAdded(QString message);
};

#endif
