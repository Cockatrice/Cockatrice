#include "signalhandler.h"

#include "main.h"
#include "server_logger.h"
#include "settingscache.h"

#include <QSocketNotifier>

#ifdef Q_OS_UNIX
#include <cstdio>
#include <execinfo.h>
#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#define SIGSEGV_TRACE_LINES 40

int SignalHandler::sigHupFD[2];

SignalHandler::SignalHandler(QObject *parent) : QObject(parent), snHup(nullptr)
{
#ifdef Q_OS_UNIX
    ::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupFD);

    snHup = new QSocketNotifier(sigHupFD[1], QSocketNotifier::Read, this);
    connect(snHup, SIGNAL(activated(int)), this, SLOT(internalSigHupHandler()));

    struct sigaction hup;
    hup.sa_handler = SignalHandler::sigHupHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &hup, 0);

    struct sigaction segv;
    segv.sa_handler = SignalHandler::sigSegvHandler;
    segv.sa_flags = SA_RESETHAND;
    sigemptyset(&segv.sa_mask);
    sigaction(SIGSEGV, &segv, 0);
    sigaction(SIGABRT, &segv, 0);

    signal(SIGPIPE, SIG_IGN);
#endif
}

void SignalHandler::sigHupHandler(int /* sig */)
{
#ifdef Q_OS_UNIX
    char a = 1;
    ssize_t writeValue = ::write(sigHupFD[0], &a, sizeof(a));
    Q_UNUSED(writeValue);
#endif
}

void SignalHandler::internalSigHupHandler()
{
    snHup->setEnabled(false);
#ifdef Q_OS_UNIX
    char tmp;
    ssize_t readValue = ::read(sigHupFD[1], &tmp, sizeof(tmp));
    Q_UNUSED(readValue);

    std::cerr << "Received SIGHUP" << std::endl;
#endif
    logger->logMessage("Received SIGHUP, rotating logs and reloading configuration", this);
    logger->rotateLogs();

    settingsCache->sync();

    snHup->setEnabled(true);
}

#ifdef Q_OS_UNIX
void SignalHandler::sigSegvHandler(int sig)
{
    void *array[SIGSEGV_TRACE_LINES];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, SIGSEGV_TRACE_LINES);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    if (sig == SIGSEGV)
        logger->logMessage("CRASH: SIGSEGV");
    else if (sig == SIGABRT)
        logger->logMessage("CRASH: SIGABRT");

    logger->deleteLater();
    loggerThread->wait();
    delete loggerThread;

    raise(sig);
}
#else
void SignalHandler::sigSegvHandler(int /* sig */)
{
}
#endif
