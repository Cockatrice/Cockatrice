#ifndef COCKATRICE_SPOILER_DOWNLOADER_H
#define COCKATRICE_SPOILER_DOWNLOADER_H

#include <QObject>
#include <QProcess>
#include <QByteArray>

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
    public:
        explicit SpoilerBackgroundUpdater(QObject *apParent = nullptr);
        inline QString getCardUpdaterBinaryName() { return "oracle"; };
        QByteArray getHash(const QString fileName);
        QByteArray getHash(QByteArray data);

    private slots:
        void actDownloadFinishedSpoilersFile();
        void actCheckIfSpoilerSeasonEnabled();

    private:
        bool isSpoilerDownloadEnabled;
        QProcess *cardUpdateProcess;
        QByteArray spoilerData;
        void startSpoilerDownloadProcess(QString url, bool saveResults);
        void downloadFromURL(QUrl url, bool saveResults);
        bool saveDownloadedFile(QByteArray data);

    signals:
        void spoilersUpdatedSuccessfully();
};

#endif //COCKATRICE_SPOILER_DOWNLOADER_H