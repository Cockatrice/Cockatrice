#include "server_logger.h"
#include "settingscache.h"
#include <QSocketNotifier>
#include <QFile>
#include <QFileInfo>
#include <QDir>
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
        QFileInfo fi(logFileName);
        QDir fileDir(fi.path());
        if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
            std::cerr << "ERROR: logfile folder doesn't exist and i can't create it." << std::endl;
            logFile = 0;
            return;
        }


        logFile = new QFile(logFileName, this);
        if(!logFile->open(QIODevice::Append)) {
            std::cerr << "ERROR: can't open() logfile." << std::endl;
            delete logFile;
            logFile = 0;
            return;
        }

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
        
    //filter out all log entries based on values in configuration file
    bool shouldWeWriteLog = settingsCache->value("server/writelog",1).toBool();
    QString logFilters = settingsCache->value("server/logfilters").toString();
    QStringList listlogFilters = logFilters.split(",", QString::SkipEmptyParts); 
    bool shouldWeSkipLine = false; 
    
    if (!shouldWeWriteLog)
        return;

    if (!logFilters.trimmed().isEmpty()){
        shouldWeSkipLine = true;
        foreach(QString logFilter, listlogFilters){ 
            if (message.contains(logFilter, Qt::CaseInsensitive)){
                shouldWeSkipLine = false;
                break;
            }
        }
    }

    if (shouldWeSkipLine)
        return;

    bufferMutex.lock();
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
    ssize_t writeValue = 0;

    if (!logFile)
        return;
    
    char a = 1;
    writeValue = ::write(sigHupFD[0], &a, sizeof(a));
#endif
}

void ServerLogger::handleSigHup()
{
#ifdef Q_OS_UNIX
    ssize_t readValue = 0;

    if (!logFile)
        return;
    
    snHup->setEnabled(false);
    char tmp;
    readValue = ::read(sigHupFD[1], &tmp, sizeof(tmp));
    
    logFile->close();
    logFile->open(QIODevice::Append);
    
    snHup->setEnabled(true);
#endif
}

QFile *ServerLogger::logFile;
int ServerLogger::sigHupFD[2];
