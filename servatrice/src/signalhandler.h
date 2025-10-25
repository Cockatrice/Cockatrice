#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <QObject>

class QSocketNotifier;

class SignalHandler : public QObject
{
    Q_OBJECT
public:
    SignalHandler(QObject *parent = 0);
    ~SignalHandler()
    {
    }
    static void sigHupHandler(int /* sig */);
    static void sigSegvHandler(int sig);

private:
    static int sigHupFD[2];
    QSocketNotifier *snHup;
private slots:
    void internalSigHupHandler();
};

#endif
