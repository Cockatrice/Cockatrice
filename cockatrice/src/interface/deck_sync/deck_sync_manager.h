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
 * (ties go to the server). Matching is path-based; a rename looks like a
 * delete plus an add. Before a download overwrites a locally modified file, the
 * local content is copied to a .sync-backup file next to it.
 *
 * Ownership: fire-and-forget per run, driven by PendingCommand round trips.
 * No watcher, no auto triggers, no conflict UI — those arrive in later layers.
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

    explicit DeckSyncManager(QObject *parent = nullptr);

    void setClient(AbstractClient *_client);
    void setProfile(const DeckSyncProfile &_profile);
    void setDeckPath(const QString &path);
    void setColorIdentityProvider(ColorIdentityProvider provider);
    [[nodiscard]] bool isSyncing() const;

public slots:
    void syncNow();

signals:
    void syncStarted();
    void statusMessage(const QString &message);
    void syncFinished(int uploaded, int downloaded, int unchanged, int skipped, const QString &error);

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
        QString serverPath;
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
    void collectServerFiles(const ServerInfo_DeckStorage_Folder &folder,
                            const QString &prefix,
                            QMap<QString, ServerFile> &files,
                            QSet<QString> &dirs);
    void processNext();
    void processNextUpload();
    void sendMkDirFor(const QString &dir);
    void sendUpload(const UploadOp &op);
    void sendDownload(const DownloadOp &op);
    void sendDelete(const QString &relativePath, int deckId);
    void snapshotLocalFile(const QString &relativePath, const QString &guid);
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
    int uploadedCount = 0;
    int downloadedCount = 0;
    int unchangedCount = 0;
    int skippedCount = 0;
};

#endif // DECK_SYNC_MANAGER_H
