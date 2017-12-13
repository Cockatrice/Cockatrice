#ifndef COCKATRICE_TIMERTHREAD_H
#define COCKATRICE_TIMERTHREAD_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>

#define GITHUB_SPOILERS_STATUS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoilerStatus.txt"
#define GITHUB_SPOILERS_DOWNLOAD_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
public:
    SpoilerBackgroundUpdater(QObject *apParent = nullptr);
    ~SpoilerBackgroundUpdater();

public slots:
    void timeoutOccurredTimeToDownloadSpoilers();
    void handleNewTimeInterval();

private slots:
    void actDownloadFinishedSpoilersFile();
    void actSeeIfSpoilerSeasonIsActive();
    void changeActiveStatus();

private:
    QTimer *mpTimerForSpoilers;
    QNetworkAccessManager *mpNetworkAccessManager;
    QByteArray mcDownloadedFileData;
    bool mbIsActiveThread;
    bool mbIsActiveSpoilerSeason;

    void runTimer(bool lbStopAndRestart = false);
    void downloadSpoilersFile(QUrl acUrl);
    void downloadSpoilerSeasonStatusFileAndStartTimer();
    bool saveSpoilersToFile(const QString &asFileName);
};


#endif //COCKATRICE_TIMERTHREAD_H
