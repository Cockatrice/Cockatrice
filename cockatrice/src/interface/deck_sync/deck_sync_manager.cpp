#include "deck_sync_manager.h"

#include "../deck_loader/deck_file_format.h"
#include "../deck_loader/deck_loader.h"
#include "deck_sync_sidecar.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QUuid>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
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
} // namespace

DeckSyncManager::DeckSyncManager(QObject *parent) : QObject(parent)
{
}

DeckSyncManager::DeckSyncManager(AbstractClient *_client, ColorIdentityProvider provider, QObject *parent)
    : QObject(parent), client(_client), colorIdentityProvider(std::move(provider))
{
}

void DeckSyncManager::setProfile(const DeckSyncProfile &_profile)
{
    profile = _profile;
}

void DeckSyncManager::setDeckPath(const QString &path)
{
    deckPath = path;
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
    unchangedCount = 0;
    skippedCount = 0;
    pendingUploads.clear();
    pendingDownloads.clear();
    ensuredDirs.clear();
    ensuredDirs.insert(QString());
    localFiles.clear();
    emit syncStarted();
    emit statusMessage(tr("Scanning local decks..."));

    sidecarPath = deckDir.filePath(SYNC_META_DIR + QString("/") + profile.key() + ".json");
    DeckSyncSidecar sidecar(sidecarPath);
    if (!sidecar.load()) {
        failSync(tr("Could not read sync state."));
        return;
    }

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
    if (response.response_code() != Response::RespOk) {
        failSync(tr("Could not read server decks."));
        return;
    }

    QMap<int, ServerFile> serverFiles;
    QSet<QString> serverDirs;
    const Response_DeckList &list = response.GetExtension(Response_DeckList::ext);
    collectServerFiles(list.root(), QString(), serverFiles, serverDirs);
    ensuredDirs.unite(serverDirs);
    emit statusMessage(tr("Comparing %1 local and %2 server decks...").arg(localFiles.size()).arg(serverFiles.size()));

    DeckSyncSidecar sidecar(sidecarPath);
    sidecar.load();
    const qint64 nowSecs = QDateTime::currentDateTime().toSecsSinceEpoch();
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

    for (auto it = localFiles.begin(); it != localFiles.end(); ++it) {
        const DeckSyncRecord sidecarRecord = sidecar.get(it.key());
        int serverId = -1;
        if (sidecarRecord.remoteId != -1 && serverFiles.contains(sidecarRecord.remoteId)) {
            serverId = sidecarRecord.remoteId;
        } else if (!sidecarRecord.guid.isEmpty() && serverIdByGuid.contains(sidecarRecord.guid)) {
            serverId = serverIdByGuid.value(sidecarRecord.guid);
        } else {
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
            ++unchangedCount;
            continue;
        }
        const qint64 localSecs = it.value().modified.toSecsSinceEpoch();
        // A server row without last_modified predates sync: the local file wins.
        // Otherwise strictly newer wins; ties go to the server.
        if (server.lastModifiedSecs == 0 || localSecs > server.lastModifiedSecs) {
            pendingUploads.append(UploadOp{.relativePath = it.key(),
                                           .serverPath = server.path,
                                           .serverDir = server.dir,
                                           .deckId = server.id,
                                           .guid = server.guid});
        } else {
            pendingDownloads.append(DownloadOp{.relativePath = it.key(),
                                               .deckId = server.id,
                                               .guid = server.guid,
                                               .hash = server.hash,
                                               .lastModifiedSecs = server.lastModifiedSecs});
        }
    }

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
        pendingDownloads.append(DownloadOp{.relativePath = relativePath,
                                           .deckId = it.value().id,
                                           .guid = it.value().guid,
                                           .hash = it.value().hash,
                                           .lastModifiedSecs = it.value().lastModifiedSecs});
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
    if (!syncing) {
        return;
    }
    if (!pendingUploads.isEmpty()) {
        processNextUpload();
    } else if (!pendingDownloads.isEmpty()) {
        sendDownload(pendingDownloads.first());
    } else {
        finishSync();
    }
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
    pend->setExtraData(op.relativePath);
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
    const QString relativePath = extraData.toString();
    if (response.response_code() != Response::RespOk) {
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
    for (int i = 0; i < pendingDownloads.size(); ++i) {
        if (pendingDownloads.at(i).relativePath == relativePath) {
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
    DeckList deck;
    if (!deck.loadFromString_Native(QString::fromStdString(download.deck()))) {
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
    if (!download.guid().empty()) {
        record.guid = QString::fromStdString(download.guid());
    }
    if (!download.hash().empty()) {
        record.baseHash = QString::fromStdString(download.hash());
    } else {
        record.baseHash = deck.getContentHash();
    }
    record.lastSyncSecs = download.has_last_modified() ? static_cast<qint64>(download.last_modified())
                                                       : QDateTime::currentDateTime().toSecsSinceEpoch();
    record.deleted = false;
    sidecar.set(relativePath, record);
    sidecar.save();
    ++downloadedCount;
    processNext();
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
    emit statusMessage(tr("Sync complete: %1 uploaded, %2 downloaded, %3 unchanged, %4 skipped.")
                           .arg(summary.uploaded)
                           .arg(summary.downloaded)
                           .arg(summary.unchanged)
                           .arg(summary.skipped));
    emit syncFinished(summary, QString());
}

DeckSyncManager::SyncSummary DeckSyncManager::currentSummary() const
{
    return SyncSummary{
        .uploaded = uploadedCount,
        .downloaded = downloadedCount,
        .unchanged = unchangedCount,
        .skipped = skippedCount,
    };
}
