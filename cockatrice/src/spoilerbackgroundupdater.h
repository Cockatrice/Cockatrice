#ifndef COCKATRICE_TIMERTHREAD_H
#define COCKATRICE_TIMERTHREAD_H

#include <QObject>
#include <QProcess>
#include <QByteArray>

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
    public:
        explicit SpoilerBackgroundUpdater(QObject *apParent = nullptr);
        inline QString getCardUpdaterBinaryName() { return "oracle"; };

    private slots:
        void actDownloadFinishedSpoilersFile();

    private:
        bool isSpoilerDownloadEnabled;
        QProcess *cardUpdateProcess;
        QByteArray spoilerData;
        void startSpoilerDownloadProcess();
        void downloadFromURL(QUrl url);
        bool saveDownloadedFile(QByteArray data);
};

#endif //COCKATRICE_TIMERTHREAD_H