#include "server_logger.h"
#include <QSocketNotifier>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#ifdef Q_OS_UNIX
# include <sys/types.h>
# include <sys/socket.h>
#endif

ServerLogger::ServerLogger(const QString &logFileName, QObject *parent)
	: QObject(parent), flushRunning(false)
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

ServerLogger::~ServerLogger()
{
	flushBuffer();
}

void ServerLogger::logMessage(QString message, void *caller)
{
	if (!logFile)
		return;
	
	bufferMutex.lock();
	QString callerString;
	if (caller)
		callerString = QString::number((qulonglong) caller, 16) + " ";
	buffer.append(QDateTime::currentDateTime().toString() + " " + QString::number((qulonglong) QThread::currentThread(), 16) + " " + callerString + message);
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
