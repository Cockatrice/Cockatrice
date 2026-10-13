#include "deck_sync_manager.h"

#include "../deck_loader/deck_loader.h"
#include "deck_sync_sidecar.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <libcockatrice/deck_list/deck_file_format.h>
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
} // namespace

DeckSyncManager::DeckSyncManager(QObject *parent) : QObject(parent)
{
}

DeckSyncManager::DeckSyncManager(AbstractClient *_client, ColorIdentityProvider provider, QObject *parent)
    : QObject(parent), client(_client), colorIdentityProvider(std::move(provider))
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
            ++unchangedCount;
            continue;
        }
        const qint64 localSecs = it.value().modified.toSecsSinceEpoch();
        // A server row without last_modified predates sync: the local file wins.
        // Otherwise strictly newer wins; ties go to the server.
        if (serverIt.value().lastModifiedSecs == 0 || localSecs > serverIt.value().lastModifiedSecs) {
            UploadOp op;
            op.relativePath = it.key();
            op.serverPath = serverPath;
            op.deckId = serverIt.value().id;
            op.guid = serverIt.value().guid;
            pendingUploads.append(op);
        } else {
            DownloadOp op;
            op.relativePath = it.key();
            op.deckId = serverIt.value().id;
            op.guid = serverIt.value().guid;
            op.hash = serverIt.value().hash;
            op.lastModifiedSecs = serverIt.value().lastModifiedSecs;
            pendingDownloads.append(op);
        }
    }

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
        record.remoteId = file.id();
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
        record.baseHash = deck.getDeckHash();
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
