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
    QByteArray getHash(const QString fileName);
    QByteArray getHash(QByteArray data);
    static bool deleteSpoilerFile();

private slots:
    void begin();
    void actDownloadFinishedSpoilersFile();
    void actCheckIfSpoilerSeasonEnabled();
    void cancelIfSettingsChanged();

private:
    enum class SaveResult
    {
        Failed,
        Unchanged,
        Written
    };

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
    QString validateDownloadedFile(const QByteArray &data) const;
    SaveResult saveDownloadedFile(const QByteArray &data, QString &error);

signals:
    void spoilersUpdatedSuccessfully();
    void spoilerCheckerDone();
    void spoilerUpdateFailed(const QString &message);
};

#endif // COCKATRICE_SPOILER_DOWNLOADER_H
