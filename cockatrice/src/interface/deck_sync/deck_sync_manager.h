/**
 * @file deck_sync_manager.h
 * @brief One-shot two-way deck collection sync between local files and the server.
 */

#ifndef DECK_SYNC_MANAGER_H
#define DECK_SYNC_MANAGER_H

#include "deck_sync_conflict.h"
#include "deck_sync_profile.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVariant>
#include <functional>
#include <libcockatrice/settings/deck_sync_settings.h>

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
 * Both-sides-changed decks pause the queue for a verdict via conflictRequest /
 * resolveConflict (or the conflict default setting); the first sync of an
 * unknown profile pauses for an onboarding verdict via firstSyncDetected /
 * continueFirstSync. No watcher and no auto triggers — those arrive later.
 *
 * Deletes are tombstones, never silent: a locally deleted deck is pushed with
 * Command_DeckDel, a server-side delete is applied locally only after copying
 * the file to a .sync-backup, and resurrection reuses the stored deck id.
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
    void setConflictDefault(int conflictDefault);
    void setSyncMode(DeckSyncMode mode);
    [[nodiscard]] bool isSyncing() const;

public slots:
    void syncNow();
    /** @brief Continues a paused first sync after the onboarding verdict. */
    void continueFirstSync(DeckSyncMode mode);
    /** @brief Aborts a paused first sync without writing any verdict. */
    void cancelFirstSync();
    /** @brief Applies the dialog verdict for the active conflict and resumes. */
    void resolveConflict(const DeckSyncResolution &resolution);

signals:
    void syncStarted();
    void statusMessage(const QString &message);
    void syncFinished(const SyncSummary &summary, const QString &error);
    /** @brief First sync with content on both sides: the tab must ask how to proceed. */
    void firstSyncDetected(int localCount, int serverCount);
    /** @brief A both-sides-changed deck needs a verdict; the queue pauses until resolveConflict. */
    void conflictRequest(const DeckSyncConflict &conflict);

private slots:
    void onListFinished(const Response &response, const CommandContainer &commandContainer);
    void
    onEnsureDirFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);
    void
    onUploadFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);
    void
    onDownloadFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);
    void
    onDeleteFinished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);

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
    struct ActiveConflict
    {
        bool active = false;
        QString relativePath;
        QString serverPath;
        int deckId = -1;
        QString serverGuid;
        QString serverHash;
        qint64 serverLastModifiedSecs = 0;
        QString serverDeckString;
    };
    struct ConflictDownload
    {
        QString serverPath;
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
    void sendDelete(const QString &relativePath, int deckId);
    void snapshotLocalFile(const QString &relativePath, const QString &guid);
    QString readBaseSnapshot(const QString &guid) const;
    void classify(const QMap<int, ServerFile> &serverFiles);
    void queueConflict(const QString &relativePath,
                       const QString &serverPath,
                       int deckId,
                       const QString &serverGuid,
                       const QString &serverHash,
                       qint64 serverLastModifiedSecs);
    void handleConflictDownload(const QString &relativePath, const QString &serverDeckString);
    void applyDownloadedDeck(const QString &relativePath,
                             int deckId,
                             const QString &deckString,
                             const QString &guid,
                             const QString &hash,
                             qint64 lastModifiedSecs,
                             bool hasLastModified);
    void applyUpload(const QString &relativePath, const QString &serverPath, int deckId, const QString &guid);
    void applyDownload(const QString &relativePath,
                       int deckId,
                       const QString &guid,
                       const QString &hash,
                       qint64 lastModifiedSecs);
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
    QList<QString> pendingDeletes;
    QSet<QString> conflictPaths;
    QMap<QString, ConflictDownload> pendingConflicts;
    QMap<int, ServerFile> stashedServerFiles;
    ActiveConflict activeConflict;
    DeckSyncMode syncMode = DeckSyncMode::TwoWay;
    int conflictDefault = DeckSyncConflictAsk;
    int sessionDefault = -1;
    bool firstSyncRun = false;
    int uploadedCount = 0;
    int downloadedCount = 0;
    int unchangedCount = 0;
    int skippedCount = 0;
};

#endif // DECK_SYNC_MANAGER_H
