/**
 * @file spoiler_background_updater.h
 * @ingroup Client
 */

#ifndef COCKATRICE_SPOILER_DOWNLOADER_H
#define COCKATRICE_SPOILER_DOWNLOADER_H

#include <QByteArray>
#include <QLoggingCategory>
#include <QObject>
#include <QPointer>
#include <QString>
#include <qtmetamacros.h>

class QNetworkAccessManager;
class QNetworkReply;
class QSystemTrayIcon;
class QUrl;

inline Q_LOGGING_CATEGORY(SpoilerBackgroundUpdaterLog, "spoiler_background_updater");

class SpoilerBackgroundUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SpoilerBackgroundUpdater(QObject *parent = nullptr, bool forceReload = false);
    inline QString getCardUpdaterBinaryName()
    {
        return "oracle";
    }
    // Queues removal and reload; the application owns the returned updater and
    // deletes it on completion. Callers can connect its failure/completion signals.
    static SpoilerBackgroundUpdater *deleteSpoilerFile();

private slots:
    void begin();
    void actDownloadFinishedSpoilersFile();
    void actCheckIfSpoilerSeasonEnabled();
    void cancelIfSettingsChanged();

private:
    const bool enabledAtStart;
    const bool forceReload;
    const QString targetPath;
    const quint64 operationId;
    QNetworkAccessManager *manager;
    QPointer<QNetworkReply> activeReply;
    QPointer<QSystemTrayIcon> notificationIcon;
    QByteArray spoilerData;
    bool completed = false;
    bool cancelled = false;
    bool reloading = false;
    bool seasonEnded = false;
    bool reloadOnly = false;

    bool isCurrent() const;
    void cancel();
    void finish(const QString &error = {}, bool updated = false);
    void downloadFromURL(const QUrl &url, bool saveResults);
    void reloadDatabase(bool updated);

signals:
    void spoilersUpdatedSuccessfully();
    void spoilerCheckerDone();
    void spoilerUpdateFailed(const QString &message);
};

#endif // COCKATRICE_SPOILER_DOWNLOADER_H
