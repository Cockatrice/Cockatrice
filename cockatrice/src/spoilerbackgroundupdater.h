#ifndef COCKATRICE_TIMERTHREAD_H
#define COCKATRICE_TIMERTHREAD_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QProcess>

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
    public:
        SpoilerBackgroundUpdater(QObject *apParent = nullptr);
        inline QString getCardUpdaterBinaryName() { return "oracle"; };

    public slots:
        void timeoutOccurredTimeToDownloadSpoilers();
        void handleNewTimeInterval();

    private slots:
        void changeActiveStatus();
        void cardUpdateError(QProcess::ProcessError err);
        void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);

    private:
        QTimer *mpTimerForSpoilers;
        bool mbIsActiveThread;
        void runTimer(bool lbStopAndRestart = false);
        void downloadSpoilersFile();
        QProcess *cardUpdateProcess;
};

#endif //COCKATRICE_TIMERTHREAD_H