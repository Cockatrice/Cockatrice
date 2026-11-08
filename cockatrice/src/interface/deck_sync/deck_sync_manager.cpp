#include "deck_sync_manager.h"

#include "../deck_loader/deck_loader.h"
#include "deck_sync_sidecar.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSaveFile>
#include <QStringList>
#include <QUuid>
#include <libcockatrice/deck_list/deck_diff.h>
#include <libcockatrice/deck_list/deck_file_format.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_del.pb.h>
#include <libcockatrice/protocol/pb/command_deck_download.pb.h>
#include <libcockatrice/protocol/pb/command_deck_list.pb.h>
#include <libcockatrice/protocol/pb/command_deck_new_dir.pb.h>
#include <libcockatrice/protocol/pb/command_deck_upload.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pb/response_deck_list.pb.h>
#include <libcockatrice/protocol/pb/response_deck_upload.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_deckstorage.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/string_limits.h>

inline Q_LOGGING_CATEGORY(DeckSyncLog, "deck_sync");

namespace
{
constexpr const char *SYNC_META_DIR = ".sync";
constexpr const char *BACKUP_SUFFIX = ".sync-backup-";

QString serverPathFor(const QString &relativePath)
{
    QString path = relativePath;
    if (path.endsWith(".cod", Qt::CaseInsensitive)) {
        path.chop(4);
    }
    return path;
}

// Rows are named after the deck, not after the file, so a row name may contain
// characters a file name cannot ("U/W Control"). Downloads therefore rebuild the
// local path from the folder plus a sanitised row name, and never from the raw
// server-supplied path.
QString sanitizedSegment(const QString &segment)
{
    QString safe = segment;
    safe.replace('/', '-');
    safe.replace('\\', '-');
    safe = safe.trimmed();
    if (safe.isEmpty() || safe == QStringLiteral(".") || safe == QStringLiteral("..")) {
        return QStringLiteral("deck");
    }
    return safe;
}

QString localPathFor(const QString &dir, const QString &name)
{
    // Every segment is server-supplied, so each one is sanitised before it can
    // become part of a local path.
    QStringList segments;
    for (const QString &segment : dir.split('/', Qt::SkipEmptyParts)) {
        segments << sanitizedSegment(segment);
    }
    segments << sanitizedSegment(name);
    const QString path = segments.join('/');
    return path.endsWith(".cod", Qt::CaseInsensitive) ? path : path + QStringLiteral(".cod");
}

// The sidecar stores the first-sync verdict so the choice outlives the run that
// asked for it. An absent or unknown value means the default two-way sync, which
// keeps profiles written before the verdict was persisted usable.
DeckSyncMode modeForSidecar(const QString &mode)
{
    if (mode == DeckSyncSidecar::MODE_UPLOAD_ONLY) {
        return DeckSyncMode::UploadOnly;
    }
    if (mode == DeckSyncSidecar::MODE_DOWNLOAD_ONLY) {
        return DeckSyncMode::DownloadOnly;
    }
    if (mode == DeckSyncSidecar::MODE_SEPARATE) {
        return DeckSyncMode::Separate;
    }
    return DeckSyncMode::TwoWay;
}

const char *sidecarModeFor(DeckSyncMode mode)
{
    switch (mode) {
        case DeckSyncMode::UploadOnly:
            return DeckSyncSidecar::MODE_UPLOAD_ONLY;
        case DeckSyncMode::DownloadOnly:
            return DeckSyncSidecar::MODE_DOWNLOAD_ONLY;
        case DeckSyncMode::Separate:
            return DeckSyncSidecar::MODE_SEPARATE;
        case DeckSyncMode::TwoWay:
            break;
    }
    return DeckSyncSidecar::MODE_LINKED;
}
} // namespace

DeckSyncManager::DeckSyncManager(QObject *parent) : QObject(parent)
{
}

DeckSyncManager::DeckSyncManager(AbstractClient *_client, ColorIdentityProvider provider, QObject *parent)
    : DeckSyncManager(parent)
{
    client = _client;
    colorIdentityProvider = std::move(provider);
}

void DeckSyncManager::setProfile(const DeckSyncProfile &_profile)
{
    profile = _profile;
}

void DeckSyncManager::setDeckPath(const QString &path)
{
    deckPath = path;
}

void DeckSyncManager::setConflictDefault(int _conflictDefault)
{
    conflictDefault = _conflictDefault;
}

bool DeckSyncManager::isSyncing() const
{
    return syncing;
}

void DeckSyncManager::syncNow()
{
    if (syncing) {
        return;
    }
    if (client == nullptr || !profile.isValid() || deckPath.isEmpty()) {
        emit syncFinished(SyncSummary{}, tr("Not connected to a server."));
        return;
    }
    QDir deckDir(deckPath);
    if (!deckDir.exists()) {
        emit syncFinished(SyncSummary{}, tr("Deck folder not found."));
        return;
    }

    syncing = true;
    uploadedCount = 0;
    downloadedCount = 0;
    removedCount = 0;
    unchangedCount = 0;
    skippedCount = 0;
    syncNote.clear();
    pendingUploads.clear();
    pendingDownloads.clear();
    pendingDeletes.clear();
    plannedLocalPaths.clear();
    conflictPaths.clear();
    stashedServerFiles.clear();
    activeConflict = ActiveConflict();
    ensuredDirs.clear();
    ensuredDirs.insert(QString());
    localFiles.clear();
    sessionDefault = -1;
    firstSyncRun = false;
    emit syncStarted();
    emit statusMessage(tr("Scanning local decks..."));

    sidecarPath = DeckSyncSidecar::pathForDeckPath(deckPath, profile.key());
    DeckSyncSidecar sidecar(sidecarPath);
    if (!sidecar.load()) {
        failSync(tr("Could not read sync state."));
        return;
    }
    // Apply the persisted first-sync verdict: the onboarding choice lasts beyond
    // the run that asked for it, and a profile that never made one syncs both ways.
    syncMode = modeForSidecar(sidecar.mode);
    if (syncMode == DeckSyncMode::Separate) {
        failSync(tr("Sync is disabled for this server."));
        return;
    }
    firstSyncRun = !QFileInfo(sidecarPath).exists();

    QDirIterator it(deckPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString absolutePath = it.next();
        const QString relativePath = deckDir.relativeFilePath(absolutePath);
        if (relativePath.startsWith(SYNC_META_DIR + QString("/")) ||
            !absolutePath.endsWith(".cod", Qt::CaseInsensitive)) {
            continue;
        }
        std::optional<LoadedDeck> loaded = DeckLoader::loadFromFile(absolutePath, DeckFileFormat::Cockatrice);
        if (!loaded) {
            ++skippedCount;
            continue;
        }
        localFiles.insert(relativePath, LocalFile{.absolutePath = absolutePath,
                                                  .hash = loaded->deckList.getContentHash(),
                                                  .modified = QFileInfo(absolutePath).lastModified()});
    }

    Command_DeckList cmd;
    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DeckSyncManager::onListFinished);
    client->sendCommand(pend);
}

void DeckSyncManager::collectServerFiles(const ServerInfo_DeckStorage_Folder &folder,
                                         const QString &prefix,
                                         QMap<int, ServerFile> &files,
                                         QSet<QString> &dirs)
{
    for (const auto &item : folder.items()) {
        const QString name = QString::fromStdString(item.name());
        const QString path = prefix.isEmpty() ? name : prefix + "/" + name;
        if (item.has_folder()) {
            dirs.insert(path);
            collectServerFiles(item.folder(), path, files, dirs);
        } else if (item.has_file()) {
            const ServerInfo_DeckStorage_File &file = item.file();
            const int id = static_cast<int>(item.id());
            ServerFile entry{
                .id = id,
                .path = path,
                .dir = prefix,
                .name = name,
                .guid = QString::fromStdString(file.guid()),
                .hash = QString::fromStdString(file.hash()),
                .lastModifiedSecs = file.has_last_modified() ? static_cast<qint64>(file.last_modified()) : 0,
            };
            // Keyed by id: the server allows several rows with the same name in a
            // folder, and keying by path silently dropped all but one of them.
            files.insert(id, entry);
        }
    }
}

void DeckSyncManager::onListFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (!syncing) {
        return;
    }
    if (response.response_code() == Response::RespNotConnected) {
        failSync(tr("Server connection lost."));
        return;
    }
    if (response.response_code() != Response::RespOk) {
        failSync(tr("Could not read server decks."));
        return;
    }

    QMap<int, ServerFile> serverFiles;
    QSet<QString> serverDirs;
    const Response_DeckList &list = response.GetExtension(Response_DeckList::ext);
    collectServerFiles(list.root(), QString(), serverFiles, serverDirs);
    ensuredDirs.unite(serverDirs);

    if (firstSyncRun && !localFiles.isEmpty() && !serverFiles.isEmpty()) {
        stashedServerFiles = serverFiles;
        emit firstSyncDetected(localFiles.size(), serverFiles.size());
        return;
    }
    classify(serverFiles);
}

void DeckSyncManager::continueFirstSync(DeckSyncMode mode)
{
    if (!syncing || !firstSyncRun) {
        return;
    }
    firstSyncRun = false;
    DeckSyncSidecar sidecar(sidecarPath);
    if (!sidecar.load()) {
        failSync(tr("Could not read sync state."));
        return;
    }
    // Persist the verdict next to the separate marker so a later syncNow() uses
    // the same direction instead of silently reverting to two-way.
    sidecar.mode = sidecarModeFor(mode);
    sidecar.save();
    if (mode == DeckSyncMode::Separate) {
        stashedServerFiles.clear();
        failSync(tr("Sync is disabled for this server."));
        return;
    }
    syncMode = mode;
    classify(stashedServerFiles);
    stashedServerFiles.clear();
}

void DeckSyncManager::cancelFirstSync()
{
    if (!syncing || !firstSyncRun) {
        return;
    }
    firstSyncRun = false;
    stashedServerFiles.clear();
    failSync(tr("Sync cancelled."));
}

void DeckSyncManager::classify(const QMap<int, ServerFile> &serverFiles)
{
    emit statusMessage(tr("Comparing %1 local and %2 server decks...").arg(localFiles.size()).arg(serverFiles.size()));

    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    const qint64 nowSecs = QDateTime::currentDateTime().toSecsSinceEpoch();
    // A listing that comes back empty while the sidecar still remembers rows that
    // were uploaded means the server lost its deck data (a purge, or an account
    // that was re-created), not that every deck was just deleted. Obeying that
    // listing would remove the whole collection, so removals are held back.
    bool sidecarHasRemoteRows = false;
    // The records as they were before this run starts mutating them; only used to
    // judge whether an empty listing is credible.
    const QMap<QString, DeckSyncRecord> sidecarAtStart = sidecar.records();
    for (auto rit = sidecarAtStart.begin(); rit != sidecarAtStart.end(); ++rit) {
        if (rit.value().remoteId != -1) {
            sidecarHasRemoteRows = true;
            break;
        }
    }
    const bool remoteStateLost = serverFiles.isEmpty() && sidecarHasRemoteRows;
    bool heldBackRemovals = false;
    // Reuse the sidecar's guid so the server keeps recognizing the row, and mint
    // one only for a deck that has never been synced.
    auto guidFor = [&sidecar](const QString &relativePath) {
        const DeckSyncRecord record = sidecar.get(relativePath);
        return record.guid.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : record.guid;
    };

    QMap<QString, int> serverIdByPath;
    QMap<QString, int> serverIdByGuid;
    for (auto sit = serverFiles.begin(); sit != serverFiles.end(); ++sit) {
        serverIdByPath.insert(sit.value().path, sit.key());
        if (!sit.value().guid.isEmpty()) {
            serverIdByGuid.insert(sit.value().guid, sit.key());
        }
    }
    QSet<int> matchedServerIds;
    QMap<QString, int> identityByLocal;

    // Pass 1: claim every row a sidecar names before any guess runs. In a single
    // pass a content or path match could take the row another local file still
    // identifies by id or guid, turning an unchanged deck into a duplicate.
    for (auto it = localFiles.begin(); it != localFiles.end(); ++it) {
        const DeckSyncRecord sidecarRecord = sidecar.get(it.key());
        int serverId = -1;
        if (sidecarRecord.remoteId != -1 && serverFiles.contains(sidecarRecord.remoteId)) {
            serverId = sidecarRecord.remoteId;
        } else if (!sidecarRecord.guid.isEmpty() && serverIdByGuid.contains(sidecarRecord.guid)) {
            serverId = serverIdByGuid.value(sidecarRecord.guid);
        }
        if (serverId != -1 && !matchedServerIds.contains(serverId)) {
            matchedServerIds.insert(serverId);
            identityByLocal.insert(it.key(), serverId);
        }
    }

    // A local file without a usable sidecar record still usually has a server
    // counterpart: rows are named after the deck, so compare content rather than
    // file names, and only upload when no unmatched row has the same content.
    auto unmatchedByHash = [&serverFiles, &matchedServerIds](const QString &hash) {
        if (hash.isEmpty()) {
            return -1;
        }
        for (auto sit = serverFiles.begin(); sit != serverFiles.end(); ++sit) {
            if (!matchedServerIds.contains(sit.key()) && sit.value().hash == hash) {
                return sit.key();
            }
        }
        return -1;
    };

    // Pass 2: resolve the remaining files by content, then path, and queue work.
    for (auto it = localFiles.begin(); it != localFiles.end(); ++it) {
        int serverId = identityByLocal.value(it.key(), -1);
        if (serverId == -1) {
            // Content first: a row with the same deck is unambiguous, while a
            // matching file name may belong to a different deck entirely.
            serverId = unmatchedByHash(it.value().hash);
            if (serverId == -1) {
                const QString wantedPath = serverPathFor(it.key());
                if (serverIdByPath.contains(wantedPath)) {
                    const int pathId = serverIdByPath.value(wantedPath);
                    if (!matchedServerIds.contains(pathId)) {
                        serverId = pathId;
                    }
                }
            }
        }
        if (serverId == -1) {
            const DeckSyncRecord sidecarRecord = sidecar.get(it.key());
            // The sidecar knows a server row that the listing no longer has: it was
            // deleted elsewhere. Only mirror that delete when the local file is
            // still exactly what was synced - an edit made after the remote delete
            // is newer than the delete and gets uploaded again.
            const bool knownToServer = sidecarRecord.remoteId != -1 || !sidecarRecord.guid.isEmpty();
            const bool unchangedSinceSync =
                !sidecarRecord.baseHash.isEmpty() && it.value().hash == sidecarRecord.baseHash;
            if (knownToServer && unchangedSinceSync) {
                if (syncMode == DeckSyncMode::UploadOnly) {
                    continue;
                }
                if (remoteStateLost) {
                    // The listing is empty although this deck was uploaded, so the
                    // row is not known to be deleted, only missing. Keep the file
                    // and the record; re-uploading it blind could duplicate the
                    // deck once the real listing returns.
                    heldBackRemovals = true;
                    ++skippedCount;
                    continue;
                }
                const QString absolutePath = deckPath + "/" + it.key();
                const QString backupPath =
                    absolutePath + BACKUP_SUFFIX + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
                if (!QFile::copy(absolutePath, backupPath) || !QFile::remove(absolutePath)) {
                    ++skippedCount;
                    continue;
                }
                sidecar.remove(it.key());
                ++removedCount;
                continue;
            }
            if (syncMode == DeckSyncMode::DownloadOnly) {
                continue;
            }
            const QString serverPath = serverPathFor(it.key());
            pendingUploads.append(UploadOp{.relativePath = it.key(),
                                           .serverPath = serverPath,
                                           .serverDir = parentDir(serverPath),
                                           .deckId = -1,
                                           .guid = guidFor(it.key())});
            continue;
        }

        const ServerFile &server = serverFiles.value(serverId);
        matchedServerIds.insert(serverId);
        if (it.value().hash == server.hash) {
            DeckSyncRecord record = sidecar.get(it.key());
            if (record.guid.isEmpty()) {
                record.guid = server.guid;
            }
            record.remoteId = server.id;
            record.baseHash = it.value().hash;
            record.lastSyncSecs = nowSecs;
            record.deleted = false;
            sidecar.set(it.key(), record);
            snapshotLocalFile(it.key(), record.guid);
            ++unchangedCount;
            continue;
        }
        const DeckSyncRecord baseRecord = sidecar.get(it.key());
        const bool localChanged = baseRecord.baseHash.isEmpty() || baseRecord.baseHash != it.value().hash;
        // A server row without a hash predates sync: treat it as unknown rather than changed,
        // otherwise every sync re-downloads the deck.
        const bool serverChanged =
            !server.hash.isEmpty() && (baseRecord.baseHash.isEmpty() || baseRecord.baseHash != server.hash);
        if (localChanged && !serverChanged) {
            if (syncMode == DeckSyncMode::DownloadOnly) {
                continue;
            }
            applyUpload(it.key(), server.path, server.id, server.guid);
            continue;
        }
        if (!localChanged && serverChanged) {
            if (syncMode == DeckSyncMode::UploadOnly) {
                continue;
            }
            applyDownload(it.key(), server.id, server.guid, server.hash, server.lastModifiedSecs);
            continue;
        }
        // Legacy row without a hash and an untouched local copy: upload once
        // so the server stamps hash and last_modified.
        if (!localChanged && !serverChanged) {
            if (syncMode != DeckSyncMode::DownloadOnly) {
                applyUpload(it.key(), server.path, server.id, server.guid);
            }
            continue;
        }
        const qint64 localSecs = it.value().modified.toSecsSinceEpoch();
        const int effective = sessionDefault >= 0 ? sessionDefault : conflictDefault;
        // A server row without last_modified predates sync: the local file wins.
        // Otherwise strictly newer wins; ties go to the server.
        if (syncMode == DeckSyncMode::UploadOnly || effective == DeckSyncConflictKeepMine) {
            applyUpload(it.key(), server.path, server.id, server.guid);
        } else if (syncMode == DeckSyncMode::DownloadOnly || effective == DeckSyncConflictKeepServer) {
            applyDownload(it.key(), server.id, server.guid, server.hash, server.lastModifiedSecs);
        } else if (effective == DeckSyncConflictKeepNewer &&
                   (server.lastModifiedSecs == 0 || localSecs > server.lastModifiedSecs)) {
            applyUpload(it.key(), server.path, server.id, server.guid);
        } else if (effective == DeckSyncConflictKeepNewer) {
            applyDownload(it.key(), server.id, server.guid, server.hash, server.lastModifiedSecs);
        } else {
            queueConflict(it.key(), server.path, server.id, server.guid, server.hash, server.lastModifiedSecs);
        }
    }

    if (syncMode != DeckSyncMode::UploadOnly) {
        for (auto it = serverFiles.begin(); it != serverFiles.end(); ++it) {
            if (matchedServerIds.contains(it.key())) {
                continue;
            }
            const QString wantedPath = localPathFor(it.value().dir, it.value().name);
            const QString relativePath = uniqueLocalPath(wantedPath);
            if (relativePath.isEmpty()) {
                ++skippedCount;
                continue;
            }
            plannedLocalPaths.insert(relativePath);
            pendingDownloads.append(DownloadOp{.relativePath = relativePath,
                                               .deckId = it.value().id,
                                               .guid = it.value().guid,
                                               .hash = it.value().hash,
                                               .lastModifiedSecs = it.value().lastModifiedSecs});
        }
    }

    if (syncMode != DeckSyncMode::DownloadOnly) {
        const QMap<QString, DeckSyncRecord> knownRecords = sidecar.records();
        for (auto it = knownRecords.begin(); it != knownRecords.end(); ++it) {
            if (localFiles.contains(it.key())) {
                continue;
            }
            if (QFileInfo::exists(deckPath + "/" + it.key())) {
                continue;
            }
            if (it.value().remoteId == -1) {
                // Never uploaded, so there is no server row to remove.
                sidecar.remove(it.key());
                continue;
            }
            if (matchedServerIds.contains(it.value().remoteId)) {
                // Another local file (a rename or a copy) now owns this row: drop the
                // stale record, but never tombstone the row it took over.
                sidecar.remove(it.key());
                continue;
            }
            if (remoteStateLost) {
                // The whole listing is missing, not this one deck: pushing the delete
                // would purge rows that are probably still there.
                heldBackRemovals = true;
                continue;
            }
            pendingDeletes.append(it.key());
        }
    }

    if (heldBackRemovals) {
        syncNote = tr("The server listed no decks although this collection was synced before. "
                      "No deletions were applied this run; the decks were kept.");
    }

    sidecar.save();
    processNext();
}

QString DeckSyncManager::parentDir(const QString &path)
{
    const int slash = path.lastIndexOf('/');
    return slash == -1 ? QString() : path.left(slash);
}

void DeckSyncManager::processNext()
{
    if (!syncing || activeConflict.active) {
        return;
    }
    if (!pendingDeletes.isEmpty()) {
        const QString relativePath = pendingDeletes.first();
        DeckSyncSidecar sidecar(sidecarPath);
        sidecar.load();
        sendDelete(relativePath, sidecar.get(relativePath).remoteId);
    } else if (!pendingUploads.isEmpty()) {
        processNextUpload();
    } else if (!pendingDownloads.isEmpty()) {
        sendDownload(pendingDownloads.first());
    } else {
        finishSync();
    }
}

void DeckSyncManager::applyUpload(const QString &relativePath,
                                  const QString &serverPath,
                                  int deckId,
                                  const QString &guid)
{
    const UploadOp op{
        .relativePath = relativePath, .serverPath = serverPath, .serverDir = {}, .deckId = deckId, .guid = guid};
    pendingUploads.append(op);
}

void DeckSyncManager::applyDownload(const QString &relativePath,
                                    int deckId,
                                    const QString &guid,
                                    const QString &hash,
                                    qint64 lastModifiedSecs)
{
    const DownloadOp op{.relativePath = relativePath,
                        .deckId = deckId,
                        .guid = guid,
                        .hash = hash,
                        .lastModifiedSecs = lastModifiedSecs};
    pendingDownloads.append(op);
}

QString DeckSyncManager::readBaseSnapshot(const QString &guid) const
{
    if (guid.isEmpty()) {
        return {};
    }
    QFile snapshot(deckPath + "/" + SYNC_META_DIR + "/bases/" + guid + ".cod");
    if (!snapshot.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromUtf8(snapshot.readAll());
}

void DeckSyncManager::queueConflict(const QString &relativePath,
                                    const QString &serverPath,
                                    int deckId,
                                    const QString &serverGuid,
                                    const QString &serverHash,
                                    qint64 serverLastModifiedSecs)
{
    // No base snapshot (the usual first two-way sync) is not a reason to pick a
    // winner silently: the dialog handles hasBase == false by disabling merge,
    // and Keep both still works, so queue the conflict like any other.
    const ConflictDownload ctx{.serverPath = serverPath,
                               .deckId = deckId,
                               .guid = serverGuid,
                               .hash = serverHash,
                               .lastModifiedSecs = serverLastModifiedSecs};
    pendingConflicts.insert(relativePath, ctx);
    conflictPaths.insert(relativePath);
    applyDownload(relativePath, deckId, serverGuid, serverHash, serverLastModifiedSecs);
}

void DeckSyncManager::handleConflictDownload(const QString &relativePath, const QString &serverDeckString)
{
    const ConflictDownload ctx = pendingConflicts.take(relativePath);
    if (sessionDefault == DeckSyncConflictKeepMine) {
        applyUpload(relativePath, ctx.serverPath, ctx.deckId, ctx.guid);
        processNext();
        return;
    }
    if (sessionDefault == DeckSyncConflictKeepServer) {
        applyDownloadedDeck(relativePath, ctx.deckId, serverDeckString, ctx.guid, ctx.hash, ctx.lastModifiedSecs,
                            ctx.lastModifiedSecs != 0);
        return;
    }
    std::optional<LoadedDeck> loaded =
        DeckLoader::loadFromFile(localFiles.value(relativePath).absolutePath, DeckFileFormat::Cockatrice);
    DeckList serverDeck;
    if (!loaded || !serverDeck.loadFromString_Native(serverDeckString)) {
        ++skippedCount;
        processNext();
        return;
    }

    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    const QString guid = !ctx.guid.isEmpty() ? ctx.guid : sidecar.get(relativePath).guid;
    activeConflict.active = true;
    activeConflict.relativePath = relativePath;
    activeConflict.serverPath = ctx.serverPath;
    activeConflict.deckId = ctx.deckId;
    activeConflict.serverGuid = ctx.guid;
    activeConflict.serverHash = ctx.hash;
    activeConflict.serverLastModifiedSecs = ctx.lastModifiedSecs;
    activeConflict.serverDeckString = serverDeckString;

    DeckSyncConflict conflict;
    conflict.relativePath = relativePath;
    conflict.localAbsolutePath = localFiles.value(relativePath).absolutePath;
    conflict.serverDeckString = serverDeckString;
    conflict.baseDeckString = readBaseSnapshot(guid);
    conflict.hasBase = !conflict.baseDeckString.isEmpty();
    conflict.localSecs = localFiles.value(relativePath).modified.toSecsSinceEpoch();
    conflict.serverSecs = ctx.lastModifiedSecs;
    emit conflictRequest(conflict);
}

void DeckSyncManager::resolveConflict(const DeckSyncResolution &resolution)
{
    if (!syncing || !activeConflict.active) {
        return;
    }
    // "Always use this rule" also settles the conflicts already queued for this
    // run, not just the setting for future ones.
    const bool remember = resolution.rememberSession || resolution.rememberAlways;
    if (remember && (resolution.action == DeckSyncResolutionAction::KeepMine ||
                     resolution.action == DeckSyncResolutionAction::KeepServer)) {
        sessionDefault = resolution.action == DeckSyncResolutionAction::KeepMine ? DeckSyncConflictKeepMine
                                                                                 : DeckSyncConflictKeepServer;
    }
    const ActiveConflict conflict = activeConflict;
    activeConflict.active = false;

    switch (resolution.action) {
        case DeckSyncResolutionAction::KeepMine:
            applyUpload(conflict.relativePath, conflict.serverPath, conflict.deckId, conflict.serverGuid);
            break;
        case DeckSyncResolutionAction::KeepServer:
            applyDownload(conflict.relativePath, conflict.deckId, conflict.serverGuid, conflict.serverHash,
                          conflict.serverLastModifiedSecs);
            break;
        case DeckSyncResolutionAction::KeepBoth: {
            // Persist the already-fetched server bytes synchronously: queuing a
            // download here would race the upload below and fetch back Mine.
            // The aside copy is a new local file, so it gets a fresh guid and
            // uploads as its own deck on the next run.
            QString asideBase = conflict.relativePath;
            if (asideBase.endsWith(".cod", Qt::CaseInsensitive)) {
                asideBase.chop(4);
            }
            const QString aside = uniqueLocalPath(asideBase + " (server).cod");
            DeckList serverDeck;
            if (aside.isEmpty() || !serverDeck.loadFromString_Native(conflict.serverDeckString)) {
                emit statusMessage(tr("Could not store the server version of %1 aside, keeping the local deck.")
                                       .arg(conflict.relativePath));
                ++skippedCount;
            } else {
                const QString asideAbsolute = deckPath + "/" + aside;
                QDir().mkpath(QFileInfo(asideAbsolute).absolutePath());
                if (!DeckLoader::saveToFile(serverDeck, asideAbsolute, DeckFileFormat::Cockatrice)) {
                    ++skippedCount;
                } else {
                    DeckSyncSidecar sidecar(sidecarPath);
                    sidecar.load();
                    DeckSyncRecord asideRecord;
                    asideRecord.guid = QUuid::createUuid().toString(QUuid::WithoutBraces);
                    asideRecord.baseHash = serverDeck.getDeckHash();
                    sidecar.set(aside, asideRecord);
                    sidecar.save();
                    snapshotLocalFile(aside, asideRecord.guid);
                }
            }
            applyUpload(conflict.relativePath, conflict.serverPath, conflict.deckId, conflict.serverGuid);
            break;
        }
        case DeckSyncResolutionAction::UseMerge: {
            DeckSyncSidecar sidecar(sidecarPath);
            sidecar.load();
            const QString guid =
                !conflict.serverGuid.isEmpty() ? conflict.serverGuid : sidecar.get(conflict.relativePath).guid;
            DeckList localDeck;
            DeckList serverDeck;
            DeckList baseDeck;
            std::optional<LoadedDeck> loaded = DeckLoader::loadFromFile(
                localFiles.value(conflict.relativePath).absolutePath, DeckFileFormat::Cockatrice);
            if (!loaded || !serverDeck.loadFromString_Native(conflict.serverDeckString) ||
                !baseDeck.loadFromString_Native(readBaseSnapshot(guid))) {
                emit statusMessage(
                    tr("Could not build the merge for %1, keeping the local deck.").arg(conflict.relativePath));
                applyUpload(conflict.relativePath, conflict.serverPath, conflict.deckId, conflict.serverGuid);
                break;
            }
            localDeck = loaded->deckList;
            const DeckDiffResult localChanges = DeckDiff::diff(baseDeck, localDeck);
            const DeckDiffResult remoteChanges = DeckDiff::diff(baseDeck, serverDeck);
            const DeckList merged = DeckDiff::merge(baseDeck, localChanges, remoteChanges);
            const QString absolutePath = deckPath + "/" + conflict.relativePath;
            const QString backupPath =
                absolutePath + BACKUP_SUFFIX + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
            if (!QFile::copy(absolutePath, backupPath) ||
                !DeckLoader::saveToFile(merged, absolutePath, DeckFileFormat::Cockatrice)) {
                ++skippedCount;
                break;
            }
            std::optional<LoadedDeck> reloaded = DeckLoader::loadFromFile(absolutePath, DeckFileFormat::Cockatrice);
            if (reloaded) {
                localFiles[conflict.relativePath].hash = reloaded->deckList.getDeckHash();
                localFiles[conflict.relativePath].modified = QFileInfo(absolutePath).lastModified();
            }
            applyUpload(conflict.relativePath, conflict.serverPath, conflict.deckId, conflict.serverGuid);
            break;
        }
        case DeckSyncResolutionAction::SkipConflict:
            ++skippedCount;
            break;
    }
    processNext();
}

void DeckSyncManager::processNextUpload()
{
    const UploadOp op = pendingUploads.first();
    const QString dir = op.serverDir;
    if (!ensuredDirs.contains(dir)) {
        sendMkDirFor(dir);
        return;
    }
    pendingUploads.removeFirst();
    sendUpload(op);
}

void DeckSyncManager::sendMkDirFor(const QString &dir)
{
    QStringList parts = dir.split('/');
    QString made;
    for (const QString &part : parts) {
        const QString candidate = made.isEmpty() ? part : made + "/" + part;
        if (!ensuredDirs.contains(candidate)) {
            Command_DeckNewDir cmd;
            cmd.set_path(made.toStdString());
            cmd.set_dir_name(part.toStdString());
            PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
            pend->setExtraData(dir);
            connect(pend, &PendingCommand::finished, this, &DeckSyncManager::onEnsureDirFinished);
            client->sendCommand(pend);
            return;
        }
        made = candidate;
    }
    processNextUpload();
}

void DeckSyncManager::onEnsureDirFinished(const Response &response,
                                          const CommandContainer & /*commandContainer*/,
                                          const QVariant &extraData)
{
    if (!syncing) {
        return;
    }
    if (response.response_code() == Response::RespNotConnected) {
        failSync(tr("Server connection lost."));
        return;
    }
    if (response.response_code() != Response::RespOk) {
        failSync(tr("Could not create server folder."));
        return;
    }
    const QString dir = extraData.toString();
    QStringList parts = dir.split('/');
    QString made;
    for (const QString &part : parts) {
        made = made.isEmpty() ? part : made + "/" + part;
        if (!ensuredDirs.contains(made)) {
            ensuredDirs.insert(made);
            break;
        }
    }
    processNextUpload();
}

void DeckSyncManager::sendUpload(const UploadOp &op)
{
    emit statusMessage(tr("Uploading %1...").arg(op.relativePath));
    std::optional<LoadedDeck> loaded =
        DeckLoader::loadFromFile(localFiles.value(op.relativePath).absolutePath, DeckFileFormat::Cockatrice);
    if (!loaded) {
        ++skippedCount;
        processNext();
        return;
    }
    // Collection sync compares and uploads the same timestamp-free string, so the
    // server's contentHash() of what it stores matches getContentHash() locally.
    const QString deckString = loaded->deckList.writeToString_Native(false);
    if (deckString.length() > MAX_FILE_LENGTH) {
        ++skippedCount;
        processNext();
        return;
    }

    Command_DeckUpload cmd;
    if (op.deckId == -1) {
        cmd.set_path(op.serverDir.toStdString());
    } else {
        cmd.set_deck_id(op.deckId);
    }
    cmd.set_deck_list(deckString.toStdString());
    cmd.set_guid(op.guid.toStdString());
    if (colorIdentityProvider) {
        cmd.set_color_identity(colorIdentityProvider(loaded->deckList).toStdString());
    }
    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    QVariantMap context;
    context.insert("path", op.relativePath);
    context.insert("serverPath", op.serverPath);
    context.insert("deckId", op.deckId);
    context.insert("guid", op.guid);
    pend->setExtraData(context);
    connect(pend, &PendingCommand::finished, this, &DeckSyncManager::onUploadFinished);
    client->sendCommand(pend);
}

void DeckSyncManager::onUploadFinished(const Response &response,
                                       const CommandContainer & /*commandContainer*/,
                                       const QVariant &extraData)
{
    if (!syncing) {
        return;
    }
    if (response.response_code() == Response::RespNotConnected) {
        failSync(tr("Server connection lost."));
        return;
    }
    const QVariantMap context = extraData.toMap();
    const QString relativePath = context.value("path").toString();
    const int deckId = context.value("deckId", -1).toInt();
    if (response.response_code() != Response::RespOk) {
        if (response.response_code() == Response::RespNameNotFound && deckId != -1) {
            // The server row is gone (purged tombstone or removed elsewhere):
            // upload the deck as new instead of updating a dead id.
            const UploadOp retry{.relativePath = relativePath,
                                 .serverPath = context.value("serverPath").toString(),
                                 .serverDir = parentDir(context.value("serverPath").toString()),
                                 .deckId = -1,
                                 .guid = context.value("guid").toString()};
            pendingUploads.prepend(retry);
            processNext();
            return;
        }
        ++skippedCount;
        processNext();
        return;
    }

    const Response_DeckUpload &upload = response.GetExtension(Response_DeckUpload::ext);
    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    DeckSyncRecord record = sidecar.get(relativePath);
    if (upload.has_new_file()) {
        const auto &file = upload.new_file();
        if (!file.file().guid().empty()) {
            record.guid = QString::fromStdString(file.file().guid());
        }
        if (!file.file().hash().empty()) {
            record.baseHash = QString::fromStdString(file.file().hash());
        }
        if (file.file().has_last_modified()) {
            record.lastSyncSecs = static_cast<qint64>(file.file().last_modified());
        } else {
            record.lastSyncSecs = QDateTime::currentDateTime().toSecsSinceEpoch();
        }
        record.remoteId = static_cast<int>(file.id());
    } else if (localFiles.contains(relativePath)) {
        record.baseHash = localFiles.value(relativePath).hash;
        record.lastSyncSecs = QDateTime::currentDateTime().toSecsSinceEpoch();
    }
    record.deleted = false;
    sidecar.set(relativePath, record);
    sidecar.save();
    snapshotLocalFile(relativePath, record.guid);
    ++uploadedCount;
    processNext();
}

QString DeckSyncManager::uniqueLocalPath(const QString &relativePath) const
{
    // Planned paths are not on disk yet, so the set keeps two downloads from
    // being queued into the same file during one run.
    auto isTaken = [this](const QString &path) {
        return plannedLocalPaths.contains(path) || QFileInfo(deckPath + "/" + path).exists();
    };
    if (!isTaken(relativePath)) {
        return relativePath;
    }
    QString base = relativePath;
    if (base.endsWith(".cod", Qt::CaseInsensitive)) {
        base.chop(4);
    }
    for (int i = 2; i < 1000; ++i) {
        const QString candidate = base + " (" + QString::number(i) + ").cod";
        if (!isTaken(candidate)) {
            return candidate;
        }
    }
    return QString();
}

void DeckSyncManager::sendDownload(const DownloadOp &op)
{
    emit statusMessage(tr("Downloading %1...").arg(op.relativePath));
    Command_DeckDownload cmd;
    cmd.set_deck_id(op.deckId);
    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    pend->setExtraData(op.relativePath);
    connect(pend, &PendingCommand::finished, this, &DeckSyncManager::onDownloadFinished);
    client->sendCommand(pend);
}

void DeckSyncManager::onDownloadFinished(const Response &response,
                                         const CommandContainer & /*commandContainer*/,
                                         const QVariant &extraData)
{
    if (!syncing) {
        return;
    }
    const QString relativePath = extraData.toString();
    int finishedDeckId = -1;
    for (int i = 0; i < pendingDownloads.size(); ++i) {
        if (pendingDownloads.at(i).relativePath == relativePath) {
            finishedDeckId = pendingDownloads.at(i).deckId;
            pendingDownloads.removeAt(i);
            break;
        }
    }
    if (response.response_code() == Response::RespNotConnected) {
        failSync(tr("Server connection lost."));
        return;
    }
    if (response.response_code() != Response::RespOk) {
        ++skippedCount;
        processNext();
        return;
    }

    const Response_DeckDownload &download = response.GetExtension(Response_DeckDownload::ext);
    if (conflictPaths.contains(relativePath)) {
        conflictPaths.remove(relativePath);
        handleConflictDownload(relativePath, QString::fromStdString(download.deck()));
        return;
    }
    applyDownloadedDeck(relativePath, finishedDeckId, QString::fromStdString(download.deck()),
                        QString::fromStdString(download.guid()), QString::fromStdString(download.hash()),
                        static_cast<qint64>(download.last_modified()), download.has_last_modified());
}

void DeckSyncManager::applyDownloadedDeck(const QString &relativePath,
                                          int deckId,
                                          const QString &deckString,
                                          const QString &guid,
                                          const QString &hash,
                                          qint64 lastModifiedSecs,
                                          bool hasLastModified)
{
    DeckList deck;
    if (!deck.loadFromString_Native(deckString)) {
        ++skippedCount;
        processNext();
        return;
    }

    const QString cleanDeckPath = QDir::cleanPath(deckPath);
    const QString absolutePath = QDir::cleanPath(deckPath + "/" + relativePath);
    if (absolutePath != cleanDeckPath && !absolutePath.startsWith(cleanDeckPath + "/")) {
        ++skippedCount;
        processNext();
        return;
    }
    if (localFiles.contains(relativePath)) {
        const QString backupPath =
            absolutePath + BACKUP_SUFFIX + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
        if (!QFile::copy(absolutePath, backupPath)) {
            ++skippedCount;
            processNext();
            return;
        }
    }
    QDir().mkpath(QFileInfo(absolutePath).absolutePath());
    if (!DeckLoader::saveToFile(deck, absolutePath, DeckFileFormat::Cockatrice)) {
        ++skippedCount;
        processNext();
        return;
    }

    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    DeckSyncRecord record = sidecar.get(relativePath);
    if (deckId != -1) {
        record.remoteId = deckId;
    }
    if (!guid.isEmpty()) {
        record.guid = guid;
    }
    if (!hash.isEmpty()) {
        record.baseHash = hash;
    } else {
        record.baseHash = deck.getContentHash();
    }
    record.lastSyncSecs = hasLastModified ? lastModifiedSecs : QDateTime::currentDateTime().toSecsSinceEpoch();
    record.deleted = false;
    sidecar.set(relativePath, record);
    sidecar.save();
    snapshotLocalFile(relativePath, record.guid);
    ++downloadedCount;
    processNext();
}

void DeckSyncManager::sendDelete(const QString &relativePath, int deckId)
{
    emit statusMessage(tr("Deleting %1 from server...").arg(relativePath));
    Command_DeckDel cmd;
    cmd.set_deck_id(deckId);
    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    pend->setExtraData(relativePath);
    connect(pend, &PendingCommand::finished, this, &DeckSyncManager::onDeleteFinished);
    client->sendCommand(pend);
}

void DeckSyncManager::onDeleteFinished(const Response &response,
                                       const CommandContainer & /*commandContainer*/,
                                       const QVariant &extraData)
{
    if (!syncing) {
        return;
    }
    const QString relativePath = extraData.toString();
    for (int i = 0; i < pendingDeletes.size(); ++i) {
        if (pendingDeletes.at(i) == relativePath) {
            pendingDeletes.removeAt(i);
            break;
        }
    }
    // Both outcomes converge the record: pushed deletes and server-side
    // deletes (already tombstoned, answering NameNotFound) are done.
    if (response.response_code() == Response::RespOk || response.response_code() == Response::RespNameNotFound) {
        DeckSyncSidecar sidecar(sidecarPath);
        sidecar.load();
        const int deletedDeckId = sidecar.get(relativePath).remoteId;
        sidecar.remove(relativePath);
        sidecar.save();
        if (deletedDeckId != -1) {
            // The unmatched-server-rows pass queued a download for this row before
            // the delete ran. The delete won, so drop the download instead of
            // letting it fail with NameNotFound and count as a skipped deck. A
            // failed delete leaves the download in place as the fallback.
            pendingDownloads.removeIf([deletedDeckId](const DownloadOp &op) { return op.deckId == deletedDeckId; });
        }
        ++removedCount;
    } else {
        ++skippedCount;
    }
    processNext();
}

void DeckSyncManager::snapshotLocalFile(const QString &relativePath, const QString &guid)
{
    if (guid.isEmpty()) {
        return;
    }
    QFile source(deckPath + "/" + relativePath);
    if (!source.open(QIODevice::ReadOnly)) {
        return;
    }
    QDir().mkpath(deckPath + "/" + SYNC_META_DIR + "/bases");
    // QSaveFile: a crash or a full disk must not leave a truncated base behind,
    // because DeckDiff::merge() against a partial base re-adds or drops cards.
    QSaveFile snapshot(deckPath + "/" + SYNC_META_DIR + "/bases/" + guid + ".cod");
    if (!snapshot.open(QIODevice::WriteOnly)) {
        return;
    }
    snapshot.write(source.readAll());
    if (!snapshot.commit()) {
        qCWarning(DeckSyncLog) << "DeckSync: could not write the base snapshot for" << relativePath;
    }
}

void DeckSyncManager::failSync(const QString &error)
{
    syncing = false;
    emit syncFinished(currentSummary(), error);
}

void DeckSyncManager::finishSync()
{
    syncing = false;
    const SyncSummary summary = currentSummary();
    QString message = tr("Sync complete: %1 uploaded, %2 downloaded, %3 removed, %4 unchanged, %5 skipped.")
                          .arg(summary.uploaded)
                          .arg(summary.downloaded)
                          .arg(summary.removed)
                          .arg(summary.unchanged)
                          .arg(summary.skipped);
    if (!summary.note.isEmpty()) {
        message += QStringLiteral(" ") + summary.note;
    }
    emit statusMessage(message);
    emit syncFinished(summary, QString());
}

DeckSyncManager::SyncSummary DeckSyncManager::currentSummary() const
{
    return SyncSummary{
        .uploaded = uploadedCount,
        .downloaded = downloadedCount,
        .removed = removedCount,
        .unchanged = unchangedCount,
        .skipped = skippedCount,
        .note = syncNote,
    };
}
