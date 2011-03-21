#include "server_logger.h"
#include <QSocketNotifier>
#include <QFile>
#include <QTextStream>
#include <sys/types.h>
#include <sys/socket.h>

ServerLogger::ServerLogger(QObject *parent)
	: QObject(parent)
{
	logFile = new QFile("server.log", this);
	logFile->open(QIODevice::Append);
#ifdef Q_OS_UNIX
	::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupFD);
#endif
	snHup = new QSocketNotifier(sigHupFD[1], QSocketNotifier::Read, this);
	connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
}

ServerLogger::~ServerLogger()
{
}

void ServerLogger::logMessage(QString message)
{
	QTextStream stream(logFile);
	stream << message << "\n";
}

void ServerLogger::hupSignalHandler(int /*unused*/)
{
	char a = 1;
	::write(sigHupFD[0], &a, sizeof(a));
}

void ServerLogger::handleSigHup()
{
	snHup->setEnabled(false);
	char tmp;
	::read(sigHupFD[1], &tmp, sizeof(tmp));
	
	logFile->close();
	logFile->open(QIODevice::Append);
	
	snHup->setEnabled(true);
}

QFile *ServerLogger::logFile;
int ServerLogger::sigHupFD[2];
