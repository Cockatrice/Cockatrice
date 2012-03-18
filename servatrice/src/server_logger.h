#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>

class QSocketNotifier;
class QFile;
class Server_ProtocolHandler;

class ServerLogger : public QObject {
	Q_OBJECT
public:
	ServerLogger(bool _logToConsole, QObject *parent = 0);
	~ServerLogger();
	static void hupSignalHandler(int unused);
public slots:
	void startLog(const QString &logFileName);
	void logMessage(QString message, void *caller = 0);
private slots:
	void handleSigHup();
	void flushBuffer();
signals:
	void sigFlushBuffer();
private:
	bool logToConsole;
	static int sigHupFD[2];
	QSocketNotifier *snHup;
	static QFile *logFile;
	bool flushRunning;
	QStringList buffer;
	QMutex bufferMutex;
};

#endif
