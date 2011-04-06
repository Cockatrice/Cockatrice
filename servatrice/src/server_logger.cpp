#include "server_logger.h"
#include <QSocketNotifier>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <sys/types.h>
#include <sys/socket.h>

ServerLogger::ServerLogger(const QString &logFileName, QObject *parent)
	: QObject(parent), flushRunning(false)
{
	if (!logFileName.isEmpty()) {
		logFile = new QFile("server.log", this);
		logFile->open(QIODevice::Append);
#ifdef Q_OS_UNIX
		::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupFD);
#endif
		snHup = new QSocketNotifier(sigHupFD[1], QSocketNotifier::Read, this);
		connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
	} else
		logFile = 0;
	
	connect(this, SIGNAL(sigFlushBuffer()), this, SLOT(flushBuffer()), Qt::QueuedConnection);
}

ServerLogger::~ServerLogger()
{
}

void ServerLogger::logMessage(QString message)
{
	if (!logFile)
		return;
	
	bufferMutex.lock();
	buffer.append(QDateTime::currentDateTime().toString() + " " + QString::number((qulonglong) QThread::currentThread(), 16) + " " + message);
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
	}
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

ServerLoggerThread::ServerLoggerThread(const QString &_fileName, QObject *parent)
	: QThread(parent), fileName(_fileName)
{
}

ServerLoggerThread::~ServerLoggerThread()
{
	quit();
	wait();
}

void ServerLoggerThread::run()
{
	logger = new ServerLogger(fileName);
	
	usleep(100);
	initWaitCondition.wakeAll();
	
	exec();
	
	delete logger;
}

void ServerLoggerThread::waitForInit()
{
	QMutex mutex;
	mutex.lock();
	initWaitCondition.wait(&mutex);
	mutex.unlock();
}
