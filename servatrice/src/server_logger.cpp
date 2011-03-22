#include "server_logger.h"
#include <QSocketNotifier>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QThread>
#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#endif

ServerLogger::ServerLogger(const QString &logFileName, QObject *parent)
	: QObject(parent)
{
	if (!logFileName.isEmpty()) {
		logFile = new QFile(logFileName, this);
		logFile->open(QIODevice::Append);
#ifdef Q_OS_UNIX
		::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupFD);
#endif
		snHup = new QSocketNotifier(sigHupFD[1], QSocketNotifier::Read, this);
		connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
	} else
		logFile = 0;
}

ServerLogger::~ServerLogger()
{
}

void ServerLogger::logMessage(QString message)
{
	if (!logFile)
		return;
	
	logFileMutex.lock();
	QTextStream stream(logFile);
	stream << QDateTime::currentDateTime().toString() << " " << ((void *) QThread::currentThread()) << " " << message << "\n";
	stream.flush();
	logFileMutex.unlock();
}

void ServerLogger::hupSignalHandler(int /*unused*/)
{
	if (!logFile)
		return;
	
	char a = 1;
	::write(sigHupFD[0], &a, sizeof(a));
}

void ServerLogger::handleSigHup()
{
	if (!logFile)
		return;
	
	snHup->setEnabled(false);
	char tmp;
	::read(sigHupFD[1], &tmp, sizeof(tmp));
	
	logFile->close();
	logFile->open(QIODevice::Append);
	
	snHup->setEnabled(true);
}

QFile *ServerLogger::logFile;
int ServerLogger::sigHupFD[2];
