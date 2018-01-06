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
        void actCheckIfSpoilerSeasonEnabled();

    private:
        bool isSpoilerDownloadEnabled;
        QProcess *cardUpdateProcess;
        QByteArray spoilerData;
        bool isSpoilerSeason;
        void startSpoilerDownloadProcess(QString url, bool saveResults);
        void downloadFromURL(QUrl url, bool saveResults);
        bool saveDownloadedFile(QByteArray data);

    signals:
        void spoilersUpdatedSuccessfully();
};

#endif //COCKATRICE_TIMERTHREAD_H