#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include <QObject>

class QSocketNotifier;
class QFile;

class ServerLogger : public QObject {
	Q_OBJECT
public:
	ServerLogger(const QString &logFileName, QObject *parent = 0);
	~ServerLogger();
	static void hupSignalHandler(int unused);
public slots:
	void logMessage(QString message);
private slots:
	void handleSigHup();
private:
	static int sigHupFD[2];
	QSocketNotifier *snHup;
	static QFile *logFile;
};

#endif
