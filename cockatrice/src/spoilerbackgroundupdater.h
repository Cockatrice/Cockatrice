#ifndef COCKATRICE_TIMERTHREAD_H
#define COCKATRICE_TIMERTHREAD_H

#include <QObject>
#include <QProcess>

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
    public:
        explicit SpoilerBackgroundUpdater(QObject *apParent = nullptr);
        inline QString getCardUpdaterBinaryName() { return "oracle"; };

    private slots:
        void cardUpdateError(QProcess::ProcessError err);
        void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);

    private:
        bool isSpoilerDownloadEnabled;
        void downloadSpoilersFile();
        QProcess *cardUpdateProcess;
};

#endif //COCKATRICE_TIMERTHREAD_H