#include "deck_sync_manager.h"

#include "../deck_loader/deck_file_format.h"
#include "../deck_loader/deck_loader.h"
#include "deck_sync_sidecar.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <libcockatrice/deck_list/deck_diff.h>
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
} // namespace

DeckSyncManager::DeckSyncManager(QObject *parent) : QObject(parent)
{
}

void DeckSyncManager::setClient(AbstractClient *_client)
{
    client = _client;
}

void DeckSyncManager::setProfile(const DeckSyncProfile &_profile)
{
    profile = _profile;
}

void DeckSyncManager::setDeckPath(const QString &path)
{
    deckPath = path;
}

void DeckSyncManager::setColorIdentityProvider(ColorIdentityProvider provider)
{
    colorIdentityProvider = std::move(provider);
}

void DeckSyncManager::setConflictDefault(int _conflictDefault)
{
    conflictDefault = _conflictDefault;
}

void DeckSyncManager::setSyncMode(DeckSyncMode mode)
{
    syncMode = mode;
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
        emit syncFinished(0, 0, 0, 0, tr("Not connected to a server."));
        return;
    }
    QDir deckDir(deckPath);
    if (!deckDir.exists()) {
        emit syncFinished(0, 0, 0, 0, tr("Deck folder not found."));
        return;
    }

    syncing = true;
    uploadedCount = 0;
    downloadedCount = 0;
    unchangedCount = 0;
    skippedCount = 0;
    pendingUploads.clear();
    pendingDownloads.clear();
    pendingDeletes.clear();
    conflictPaths.clear();
    stashedServerFiles.clear();
    activeConflict = ActiveConflict();
    ensuredDirs.clear();
    ensuredDirs.insert(QString());
    localFiles.clear();
    sessionDefault = -1;
    syncMode = DeckSyncMode::TwoWay;
    firstSyncRun = false;
    emit syncStarted();
    emit statusMessage(tr("Scanning local decks..."));

    sidecarPath = DeckSyncSidecar::pathForDeckPath(deckPath, profile.key());
    DeckSyncSidecar sidecar(sidecarPath);
    if (!sidecar.load()) {
        failSync(tr("Could not read sync state."));
        return;
    }
    if (sidecar.mode == DeckSyncSidecar::MODE_SEPARATE) {
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
        LocalFile file;
        file.absolutePath = absolutePath;
        file.hash = loaded->deckList.getDeckHash();
        file.modified = QFileInfo(absolutePath).lastModified();
        localFiles.insert(relativePath, file);
    }

    Command_DeckList cmd;
    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DeckSyncManager::onListFinished);
    client->sendCommand(pend);
}

void DeckSyncManager::collectServerFiles(const ServerInfo_DeckStorage_Folder &folder,
                                         const QString &prefix,
                                         QMap<QString, ServerFile> &files,
                                         QSet<QString> &dirs)
{
    for (const auto &item : folder.items()) {
        const QString name = QString::fromStdString(item.name());
        if (name.isEmpty() || name == "." || name == ".." || name.contains('/') || name.contains('\\')) {
            continue;
        }
        if (item.has_folder()) {
            const QString path = prefix.isEmpty() ? name : prefix + "/" + name;
            dirs.insert(path);
            collectServerFiles(item.folder(), path, files, dirs);
        } else if (item.has_file()) {
            const QString path = prefix.isEmpty() ? name : prefix + "/" + name;
            const ServerInfo_DeckStorage_File &file = item.file();
            ServerFile entry;
            entry.id = item.id();
            entry.guid = QString::fromStdString(file.guid());
            entry.hash = QString::fromStdString(file.hash());
            entry.lastModifiedSecs = file.has_last_modified() ? static_cast<qint64>(file.last_modified()) : 0;
            files.insert(path, entry);
        }
    }
}

void DeckSyncManager::onListFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (!syncing) {
        return;
    }
    if (response.response_code() != Response::RespOk) {
        failSync(tr("Could not read server decks."));
        return;
    }

    QMap<QString, ServerFile> serverFiles;
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
    if (mode == DeckSyncMode::Separate) {
        DeckSyncSidecar sidecar(sidecarPath);
        sidecar.load();
        sidecar.mode = DeckSyncSidecar::MODE_SEPARATE;
        sidecar.save();
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

void DeckSyncManager::classify(const QMap<QString, ServerFile> &serverFiles)
{
    emit statusMessage(tr("Comparing %1 local and %2 server decks...").arg(localFiles.size()).arg(serverFiles.size()));

    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    const qint64 nowSecs = QDateTime::currentDateTime().toSecsSinceEpoch();

    QMap<int, QString> serverPathById;
    QMap<QString, QString> serverPathByGuid;
    for (auto sit = serverFiles.begin(); sit != serverFiles.end(); ++sit) {
        serverPathById.insert(sit.value().id, sit.key());
        if (!sit.value().guid.isEmpty()) {
            serverPathByGuid.insert(sit.value().guid, sit.key());
        }
    }
    QSet<QString> matchedServerPaths;

    for (auto it = localFiles.begin(); it != localFiles.end(); ++it) {
        const DeckSyncRecord sidecarRecord = sidecar.get(it.key());
        QString serverPath;
        if (sidecarRecord.remoteId != -1 && serverPathById.contains(sidecarRecord.remoteId)) {
            serverPath = serverPathById.value(sidecarRecord.remoteId);
        } else if (!sidecarRecord.guid.isEmpty() && serverPathByGuid.contains(sidecarRecord.guid)) {
            serverPath = serverPathByGuid.value(sidecarRecord.guid);
        } else {
            serverPath = serverPathFor(it.key());
            if (!serverFiles.contains(serverPath)) {
                UploadOp op;
                op.relativePath = it.key();
                op.serverPath = serverPath;
                op.deckId = -1;
                op.guid = sidecar.contains(it.key()) && !sidecar.get(it.key()).guid.isEmpty()
                              ? sidecar.get(it.key()).guid
                              : QUuid::createUuid().toString(QUuid::WithoutBraces);
                pendingUploads.append(op);
                continue;
            }
        }
        auto serverIt = serverFiles.find(serverPath);
        if (serverIt == serverFiles.end()) {
            const DeckSyncRecord known = sidecar.get(it.key());
            if (known.remoteId != -1) {
                if (syncMode == DeckSyncMode::UploadOnly) {
                    continue;
                }
                // The server no longer lists this deck: it was deleted elsewhere.
                // Apply the delete locally, keeping a backup of the file.
                const QString absolutePath = deckPath + "/" + it.key();
                const QString backupPath =
                    absolutePath + BACKUP_SUFFIX + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
                if (!QFile::copy(absolutePath, backupPath) || !QFile::remove(absolutePath)) {
                    ++skippedCount;
                    continue;
                }
                sidecar.remove(it.key());
                ++downloadedCount;
                continue;
            }
            if (syncMode == DeckSyncMode::DownloadOnly) {
                continue;
            }
            UploadOp op;
            op.relativePath = it.key();
            op.serverPath = serverPath;
            op.deckId = -1;
            op.guid = !known.guid.isEmpty() ? known.guid : QUuid::createUuid().toString(QUuid::WithoutBraces);
            pendingUploads.append(op);
            continue;
        }
        matchedServerPaths.insert(serverIt.key());
        if (it.value().hash == serverIt.value().hash) {
            DeckSyncRecord record = sidecar.get(it.key());
            if (record.guid.isEmpty()) {
                record.guid = serverIt.value().guid;
            }
            record.remoteId = serverIt.value().id;
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
        const bool serverChanged = baseRecord.baseHash.isEmpty() || baseRecord.baseHash != serverIt.value().hash;
        if (localChanged && !serverChanged) {
            if (syncMode == DeckSyncMode::DownloadOnly) {
                continue;
            }
            applyUpload(it.key(), serverPath, serverIt.value().id, serverIt.value().guid);
            continue;
        }
        if (!localChanged && serverChanged) {
            if (syncMode == DeckSyncMode::UploadOnly) {
                continue;
            }
            applyDownload(it.key(), serverIt.value().id, serverIt.value().guid, serverIt.value().hash,
                          serverIt.value().lastModifiedSecs);
            continue;
        }
        const qint64 localSecs = it.value().modified.toSecsSinceEpoch();
        const int effective = sessionDefault >= 0 ? sessionDefault : conflictDefault;
        // A server row without last_modified predates sync: the local file wins.
        // Otherwise strictly newer wins; ties go to the server.
        if (syncMode == DeckSyncMode::UploadOnly || effective == DeckSyncConflictKeepMine) {
            applyUpload(it.key(), serverPath, serverIt.value().id, serverIt.value().guid);
        } else if (syncMode == DeckSyncMode::DownloadOnly || effective == DeckSyncConflictKeepServer) {
            applyDownload(it.key(), serverIt.value().id, serverIt.value().guid, serverIt.value().hash,
                          serverIt.value().lastModifiedSecs);
        } else if (effective == DeckSyncConflictKeepNewer &&
                   (serverIt.value().lastModifiedSecs == 0 || localSecs > serverIt.value().lastModifiedSecs)) {
            applyUpload(it.key(), serverPath, serverIt.value().id, serverIt.value().guid);
        } else if (effective == DeckSyncConflictKeepNewer) {
            applyDownload(it.key(), serverIt.value().id, serverIt.value().guid, serverIt.value().hash,
                          serverIt.value().lastModifiedSecs);
        } else {
            queueConflict(it.key(), serverPath, serverIt.value().id, serverIt.value().guid, serverIt.value().hash,
                          serverIt.value().lastModifiedSecs);
        }
    }

    if (syncMode != DeckSyncMode::UploadOnly) {
        for (auto it = serverFiles.begin(); it != serverFiles.end(); ++it) {
            const QString wantedPath = it.key().endsWith(".cod", Qt::CaseInsensitive) ? it.key() : it.key() + ".cod";
            if (localFiles.contains(wantedPath) || QFileInfo(deckPath + "/" + wantedPath).exists()) {
                continue;
            }
            const QString relativePath = uniqueLocalPath(wantedPath);
            if (relativePath.isEmpty()) {
                ++skippedCount;
                continue;
            }
            DownloadOp op;
            op.relativePath = relativePath;
            op.deckId = it.value().id;
            op.guid = it.value().guid;
            op.hash = it.value().hash;
            op.lastModifiedSecs = it.value().lastModifiedSecs;
            pendingDownloads.append(op);
        }
    }

<    if (syncMode != DeckSyncMode::UploadOnly) {
        for (auto it = serverFiles.begin(); it != serverFiles.end(); ++it) {
            if (matchedServerPaths.contains(it.key())) {
                continue;
            }
            const QString wantedPath = it.key().endsWith(".cod", Qt::CaseInsensitive) ? it.key() : it.key() + ".cod";
            if (localFiles.contains(wantedPath) || QFileInfo(deckPath + "/" + wantedPath).exists()) {
                continue;
            }
            const QString relativePath = uniqueLocalPath(wantedPath);
            if (relativePath.isEmpty()) {
                ++skippedCount;
                continue;
            }
            DownloadOp op;
            op.relativePath = relativePath;
            op.deckId = it.value().id;
            op.guid = it.value().guid;
            op.hash = it.value().hash;
            op.lastModifiedSecs = it.value().lastModifiedSecs;
            pendingDownloads.append(op);
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
                sidecar.remove(it.key());
                continue;
            }
            pendingDeletes.append(it.key());
        }
    }
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
    UploadOp op;
    op.relativePath = relativePath;
    op.serverPath = serverPath;
    op.deckId = deckId;
    op.guid = guid;
    pendingUploads.append(op);
}

void DeckSyncManager::applyDownload(const QString &relativePath,
                                    int deckId,
                                    const QString &guid,
                                    const QString &hash,
                                    qint64 lastModifiedSecs)
{
    DownloadOp op;
    op.relativePath = relativePath;
    op.deckId = deckId;
    op.guid = guid;
    op.hash = hash;
    op.lastModifiedSecs = lastModifiedSecs;
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
    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    const QString guid = !serverGuid.isEmpty() ? serverGuid : sidecar.get(relativePath).guid;
    if (readBaseSnapshot(guid).isEmpty()) {
        emit statusMessage(tr("No base snapshot for %1, keeping the newer version.").arg(relativePath));
        const qint64 localSecs = localFiles.value(relativePath).modified.toSecsSinceEpoch();
        if (serverLastModifiedSecs == 0 || localSecs > serverLastModifiedSecs) {
            applyUpload(relativePath, serverPath, deckId, serverGuid);
        } else {
            applyDownload(relativePath, deckId, serverGuid, serverHash, serverLastModifiedSecs);
        }
        return;
    }
    ConflictDownload ctx;
    ctx.serverPath = serverPath;
    ctx.deckId = deckId;
    ctx.guid = serverGuid;
    ctx.hash = serverHash;
    ctx.lastModifiedSecs = serverLastModifiedSecs;
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
    if (resolution.rememberSession &&
        (resolution.action == DeckSyncKeepMine || resolution.action == DeckSyncKeepServer)) {
        sessionDefault = resolution.action == DeckSyncKeepMine ? DeckSyncConflictKeepMine : DeckSyncConflictKeepServer;
    }
    const ActiveConflict conflict = activeConflict;
    activeConflict.active = false;

    switch (resolution.action) {
        case DeckSyncKeepMine:
            applyUpload(conflict.relativePath, conflict.serverPath, conflict.deckId, conflict.serverGuid);
            break;
        case DeckSyncKeepServer:
            applyDownload(conflict.relativePath, conflict.deckId, conflict.serverGuid, conflict.serverHash,
                          conflict.serverLastModifiedSecs);
            break;
        case DeckSyncKeepBoth: {
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
        case DeckSyncUseMerge: {
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
        case DeckSyncSkipConflict:
            ++skippedCount;
            break;
    }
    processNext();
}

void DeckSyncManager::processNextUpload()
{
    const UploadOp op = pendingUploads.first();
    const QString dir = parentDir(op.serverPath);
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
    const QString deckString = loaded->deckList.writeToString_Native();
    if (deckString.length() > MAX_FILE_LENGTH) {
        ++skippedCount;
        processNext();
        return;
    }

    Command_DeckUpload cmd;
    if (op.deckId == -1) {
        cmd.set_path(parentDir(op.serverPath).toStdString());
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
    const QVariantMap context = extraData.toMap();
    const QString relativePath = context.value("path").toString();
    const int deckId = context.value("deckId", -1).toInt();
    if (response.response_code() != Response::RespOk) {
        if (response.response_code() == Response::RespNameNotFound && deckId != -1) {
            // The server row is gone (purged tombstone or removed elsewhere):
            // upload the deck as new instead of updating a dead id.
            UploadOp retry;
            retry.relativePath = relativePath;
            retry.serverPath = context.value("serverPath").toString();
            retry.deckId = -1;
            retry.guid = context.value("guid").toString();
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
        record.remoteId = file.id();
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
    if (!QFileInfo(deckPath + "/" + relativePath).exists()) {
        return relativePath;
    }
    QString base = relativePath;
    if (base.endsWith(".cod", Qt::CaseInsensitive)) {
        base.chop(4);
    }
    for (int i = 2; i < 1000; ++i) {
        const QString candidate = base + " (" + QString::number(i) + ").cod";
        if (!QFileInfo(deckPath + "/" + candidate).exists()) {
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
        record.baseHash = deck.getDeckHash();
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
        sidecar.remove(relativePath);
        sidecar.save();
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
    QFile snapshot(deckPath + "/" + SYNC_META_DIR + "/bases/" + guid + ".cod");
    if (!snapshot.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    snapshot.write(source.readAll());
}

void DeckSyncManager::failSync(const QString &error)
{
    syncing = false;
    emit syncFinished(uploadedCount, downloadedCount, unchangedCount, skippedCount, error);
}

void DeckSyncManager::finishSync()
{
    syncing = false;
    emit statusMessage(tr("Sync complete: %1 uploaded, %2 downloaded, %3 unchanged, %4 skipped.")
                           .arg(uploadedCount)
                           .arg(downloadedCount)
                           .arg(unchangedCount)
                           .arg(skippedCount));
    emit syncFinished(uploadedCount, downloadedCount, unchangedCount, skippedCount, QString());
}
