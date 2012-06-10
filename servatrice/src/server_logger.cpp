#include "server_logger.h"
#include <QSocketNotifier>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#ifdef Q_OS_UNIX
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
#endif

ServerLogger::ServerLogger(bool _logToConsole, QObject *parent)
	: QObject(parent), logToConsole(_logToConsole), flushRunning(false)
{
}

ServerLogger::~ServerLogger()
{
	flushBuffer();
	// This does not work with the destroyed() signal as this destructor is called after the main event loop is done.
	thread()->quit();
}

void ServerLogger::startLog(const QString &logFileName)
{
	if (!logFileName.isEmpty()) {
		logFile = new QFile("server.log", this);
		logFile->open(QIODevice::Append);
#ifdef Q_OS_UNIX
		::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupFD);

		snHup = new QSocketNotifier(sigHupFD[1], QSocketNotifier::Read, this);
		connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
#endif
	} else
		logFile = 0;
	
	connect(this, SIGNAL(sigFlushBuffer()), this, SLOT(flushBuffer()), Qt::QueuedConnection);
}

void ServerLogger::logMessage(QString message, void *caller)
{
	if (!logFile)
		return;
	
	bufferMutex.lock();
	QString callerString;
	if (caller)
		callerString = QString::number((qulonglong) caller, 16) + " ";
	buffer.append(QDateTime::currentDateTime().toString() + " " + callerString + message);
	bufferMutex.unlock();
	
	emit sigFlushBuffer();
}

void ServerLogger::flushBuffer()
{
	if (flushRunning)
		return;
	
	flushRunning = true;
	QTextStream stream(logFile);
	forever {
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
		
		if (logToConsole)
			std::cout << message.toStdString() << std::endl;
	}
}

void ServerLogger::hupSignalHandler(int /*unused*/)
{
#ifdef Q_OS_UNIX
	if (!logFile)
		return;
	
	char a = 1;
	::write(sigHupFD[0], &a, sizeof(a));
#endif
}

void ServerLogger::handleSigHup()
{
#ifdef Q_OS_UNIX
	if (!logFile)
		return;
	
	snHup->setEnabled(false);
	char tmp;
	::read(sigHupFD[1], &tmp, sizeof(tmp));
	
	logFile->close();
	logFile->open(QIODevice::Append);
	
	snHup->setEnabled(true);
#endif
}

QFile *ServerLogger::logFile;
int ServerLogger::sigHupFD[2];
