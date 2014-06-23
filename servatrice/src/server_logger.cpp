#include "server_logger.h"
#include <QSocketNotifier>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QSettings>
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
	
	QString callerString;
	if (caller)
		callerString = QString::number((qulonglong) caller, 16) + " ";
		
	//filter out all log entries based on loglevel value in configuration file
	QSettings *settings = new QSettings("servatrice.ini", QSettings::IniFormat);
	int found = 0; int capture = 0; int loglevel = 0;
	loglevel = settings->value("server/loglevel").toInt();
	switch (loglevel)
	{
		case 1:
			break;

		case 2:

			// filter message log data
			found = message.indexOf("Adding room: ID=", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Starting status update clock", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Starting server on port", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Server listening.", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Server::loginUser:", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Server::removeClient:", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Command_Login.ext", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Command_RoomSay.ext", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Command_Message.ext", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			found = message.indexOf("Command_GameSay.ext", Qt::CaseInsensitive); if (found != -1){ capture = 1; }
			if (capture != 0){
				bufferMutex.lock();
				buffer.append(QDateTime::currentDateTime().toString() + " " + callerString + message);
				bufferMutex.unlock();
			}
			break;

		default:
			bufferMutex.lock();
			buffer.append(QDateTime::currentDateTime().toString() + " " + callerString + message);
			bufferMutex.unlock();
	}
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
