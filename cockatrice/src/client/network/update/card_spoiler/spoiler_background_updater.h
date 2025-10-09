/**
 * @file spoiler_background_updater.h
 * @ingroup Client
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_SPOILER_DOWNLOADER_H
#define COCKATRICE_SPOILER_DOWNLOADER_H

#include <QByteArray>
#include <QLoggingCategory>
#include <QObject>
#include <QProcess>

inline Q_LOGGING_CATEGORY(SpoilerBackgroundUpdaterLog, "spoiler_background_updater");

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SpoilerBackgroundUpdater(QObject *apParent = nullptr);
    inline QString getCardUpdaterBinaryName()
    {
        return "oracle";
    };
    QByteArray getHash(const QString fileName);
    QByteArray getHash(QByteArray data);
    static bool deleteSpoilerFile();

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
    void spoilerCheckerDone();
};

#endif // COCKATRICE_SPOILER_DOWNLOADER_H