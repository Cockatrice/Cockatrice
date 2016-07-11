#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QFile>
#include <QVector>
#include <QString>

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

    QVector<QString> logBuffer;
public:
	void logToFile(bool enabled);
    void log(QtMsgType type, const QMessageLogContext &ctx, const QString &message);
    QVector<QString> getLogBuffer() { return logBuffer; }
protected:
    void openLogfileSession();
    void closeLogfileSession();
signals:
    void logEntryAdded(QString message);
};

#endif
