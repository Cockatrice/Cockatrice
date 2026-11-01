/**
 * @file deck_sync_manager.h
 * @brief One-shot two-way deck collection sync between local files and the server.
 */

#ifndef DECK_SYNC_MANAGER_H
#define DECK_SYNC_MANAGER_H

#include "deck_sync_profile.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVariant>
#include <functional>

class AbstractClient;
class CommandContainer;
class DeckList;
class Response;
class ServerInfo_DeckStorage_Folder;

/**
 * @class DeckSyncManager
 * @brief Runs a manual whole-collection sync: local *.cod files vs server deck storage.
 *
 * Each syncNow() run loads the profile sidecar, scans the deck folder, fetches
 * the server tree and mirrors both sides: unknown local decks upload, unknown
 * server decks download, and decks changed on both sides resolve newer-wins
 * (ties go to the server). A local file is matched to its server row by the
 * sidecar's row id, then guid, then the server path, and finally by content, so
 * renaming either side does not duplicate a deck. Before a download overwrites a
 * locally modified file, the local content is copied to a .sync-backup file next
 * to it.
 *
 * Ownership: fire-and-forget per run, driven by PendingCommand round trips.
 * No watcher, no auto triggers, no conflict UI — those arrive in later layers.
 */
class DeckSyncManager : public QObject
{
    Q_OBJECT
public:
    /** @brief Computes the color identity string uploaded alongside a deck. May be empty. */
    using ColorIdentityProvider = std::function<QString(const DeckList &)>;

    /** @brief Outcome counts for a finished sync run, carried by syncFinished(). */
    struct SyncSummary
    {
        int uploaded = 0;
        int downloaded = 0;
        int unchanged = 0;
        int skipped = 0;
    };

    explicit DeckSyncManager(QObject *parent = nullptr);
    DeckSyncManager(AbstractClient *client, ColorIdentityProvider provider, QObject *parent = nullptr);

    void setProfile(const DeckSyncProfile &_profile);
    void setDeckPath(const QString &path);
    [[nodiscard]] bool isSyncing() const;

public slots:
    void syncNow();

signals:
    void syncStarted();
    void statusMessage(const QString &message);
    void syncFinished(const SyncSummary &summary, const QString &error);

private slots:
    void onListFinished(const Response &response, const CommandContainer &commandContainer);
    void
    onEnsureDirFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);
    void
    onUploadFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);
    void
    onDownloadFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);

private:
    struct ServerFile
    {
        int id = -1;
        QString path; ///< Server-side address: folder prefix plus row name.
        QString dir;  ///< Folder prefix, empty in the root.
        QString name; ///< Row name exactly as the server stores it.
        QString guid;
        QString hash;
        qint64 lastModifiedSecs = 0;
    };
    struct LocalFile
    {
        QString absolutePath;
        QString hash;
        QDateTime modified;
    };
    struct UploadOp
    {
        QString relativePath;
        QString serverPath; ///< Target address of a brand-new row.
        QString serverDir;  ///< Folder that must exist before the upload.
        int deckId = -1;
        QString guid;
    };
    struct DownloadOp
    {
        QString relativePath;
        int deckId = -1;
        QString guid;
        QString hash;
        qint64 lastModifiedSecs = 0;
    };

    void failSync(const QString &error);
    void finishSync();
    [[nodiscard]] SyncSummary currentSummary() const;
    void collectServerFiles(const ServerInfo_DeckStorage_Folder &folder,
                            const QString &prefix,
                            QMap<int, ServerFile> &files,
                            QSet<QString> &dirs);
    void processNext();
    void processNextUpload();
    void sendMkDirFor(const QString &dir);
    void sendUpload(const UploadOp &op);
    void sendDownload(const DownloadOp &op);
    static QString parentDir(const QString &path);
    QString uniqueLocalPath(const QString &relativePath) const;

    AbstractClient *client = nullptr;
    DeckSyncProfile profile;
    QString deckPath;
    ColorIdentityProvider colorIdentityProvider;
    bool syncing = false;

    QString sidecarPath;
    QMap<QString, LocalFile> localFiles;
    QSet<QString> ensuredDirs;
    QList<UploadOp> pendingUploads;
    QList<DownloadOp> pendingDownloads;
    int uploadedCount = 0;
    int downloadedCount = 0;
    int unchangedCount = 0;
    int skippedCount = 0;
};

#endif // DECK_SYNC_MANAGER_H
