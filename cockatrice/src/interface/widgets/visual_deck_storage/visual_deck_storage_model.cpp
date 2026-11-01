#include "visual_deck_storage_model.h"

#include "../../deck_loader/deck_loader.h"
#include "../cards/additional_info/deck_color_identity.h"
#include "libcockatrice/deck_list/deck_list.h"

#include <QChar>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QFuture>
#include <QMetaObject>
#include <QMetaType>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <QThread>
#include <QVariant>
#include <QtConcurrentRun>
#include <compare>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_file_format.h>
#include <libcockatrice/deck_list/loaded_deck.h>
#include <libcockatrice/utility/card_ref.h>
#include <optional>
#include <utility>

class QObject;

namespace
{
/**
 * @brief The result of a background directory scan: the deck rows in scan order
 * plus the sorted list of subfolder paths.
 */
struct DeckScanResult
{
    QList<DeckPreviewData> decks; ///< Deck rows in scan order.
    QStringList folderPaths;      ///< Sorted list of subfolder paths.
};

/**
 * @brief The result of a background deck file load: the parsed deck plus the
 * file's modification time, so the disk stat happens off the UI thread.
 */
struct DeckLoadResult
{
    LoadedDeck deck;        ///< The parsed deck.
    QDateTime lastModified; ///< File modification time at load.
    QString colorIdentity;  ///< WUBRG color identity, computed off the UI thread.
};

/**
 * @brief How long per event loop turn the pending-load drain applies finished loads.
 *
 * Applying a load updates the row and wakes up the proxies and views, which is
 * not free. A time budget instead of a fixed count lets fast machines apply
 * many loads in one turn while keeping the ui thread responsive everywhere.
 */
constexpr int LOAD_DRAIN_TIME_BUDGET_MS = 8;

/**
 * @brief The path of \a path relative to the deck root, or empty if \a path
 * is not below it.
 */
QString relativePathFromDeckRoot(const QString &path, const QString &deckPath)
{
    if (!path.startsWith(deckPath)) {
        return {};
    }
    QString relativePath = path.mid(deckPath.length());
    if (relativePath.startsWith('/')) {
        relativePath.remove(0, 1);
    }
    return relativePath;
}

/**
 * @brief The path of \a filePath relative to \a deckPath, or the bare file name
 * if \a filePath is not below \a deckPath.
 */
QString relativeFilePathFor(const QString &filePath, const QString &deckPath)
{
    if (filePath.startsWith(deckPath)) {
        return filePath.mid(deckPath.length());
    }

    return QFileInfo(filePath).fileName();
}

/**
 * @brief The directory of \a filePath relative to \a deckPath, or empty if the
 * file sits directly in the deck root.
 */
QString folderPathFor(const QString &filePath, const QString &deckPath)
{
    return relativePathFromDeckRoot(QFileInfo(filePath).absolutePath(), deckPath);
}

/**
 * @brief Scans a deck directory on a worker thread, returning discovered deck
 * files and subfolder paths.
 */
DeckScanResult scanDeckDirectory(const QString &deckPath, const std::shared_ptr<std::atomic_bool> &cancelled)
{
    DeckScanResult result;

    QDirIterator fileIt(deckPath, DeckLoader::ACCEPTED_FILE_EXTENSIONS, QDir::Files,
                        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (!cancelled->load(std::memory_order_relaxed) && fileIt.hasNext()) {
        const QString filePath = fileIt.next();
        DeckPreviewData data;
        data.filePath = filePath;
        data.relativeFilePath = relativeFilePathFor(filePath, deckPath);
        data.folderPath = folderPathFor(filePath, deckPath);
        data.lastModified = QFileInfo(filePath).lastModified();
        result.decks.append(std::move(data));
    }

    if (cancelled->load(std::memory_order_relaxed)) {
        return result;
    }

    QSet<QString> seenFolders;
    QDirIterator folderIt(deckPath, QDir::Dirs | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (!cancelled->load(std::memory_order_relaxed) && folderIt.hasNext()) {
        const QString folderPath = relativePathFromDeckRoot(folderIt.next(), deckPath);
        if (!folderPath.isEmpty() && !seenFolders.contains(folderPath)) {
            seenFolders.insert(folderPath);
            result.folderPaths.append(folderPath);
        }
    }
    result.folderPaths.sort();

    return result;
}
} // namespace

VisualDeckStorageModel::VisualDeckStorageModel(QObject *parent) : QAbstractListModel(parent)
{
    // Multiple visual storage views must not each consume all host cores.
    taskPool.setMaxThreadCount(qBound(1, QThread::idealThreadCount(), 4));
}

VisualDeckStorageModel::~VisualDeckStorageModel()
{
    // Queued tasks become cheap no-ops; the pool then joins active tasks as it is destroyed.
    scanCancelled->store(true, std::memory_order_relaxed);
}

int VisualDeckStorageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : decks.size();
}

QVariant VisualDeckStorageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= decks.size()) {
        return {};
    }

    const DeckPreviewData &data = decks.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
        case VisualDeckStorageRoles::DisplayNameRole:
            return data.displayName;
        case VisualDeckStorageRoles::FilePathRole:
            return data.filePath;
        case VisualDeckStorageRoles::RelativeFilePathRole:
            return data.relativeFilePath;
        case VisualDeckStorageRoles::FolderPathRole:
            return data.folderPath;
        case VisualDeckStorageRoles::TagsRole:
            return data.tags;
        case VisualDeckStorageRoles::ColorIdentityRole:
            return data.colorIdentity;
        case VisualDeckStorageRoles::LastModifiedRole:
            return data.lastModified;
        case VisualDeckStorageRoles::LastLoadedRole:
            return data.lastLoaded;
        case VisualDeckStorageRoles::BannerCardNameRole:
            return data.bannerCard.name;
        case VisualDeckStorageRoles::BannerCardProviderIdRole:
            return data.bannerCard.providerId;
        default:
            return {};
    }
}

void VisualDeckStorageModel::setDeckPath(const QString &path)
{
    QString cleanedPath = QDir::cleanPath(path);
    if (cleanedPath == ".") {
        cleanedPath.clear();
    }
    deckPath = cleanedPath;
    startScan();
}

void VisualDeckStorageModel::refresh()
{
    startScan();
}

const DeckPreviewData &VisualDeckStorageModel::dataForRow(int row) const
{
    static const DeckPreviewData emptyData;
    if (row < 0 || row >= decks.size()) {
        return emptyData;
    }
    return decks.at(row);
}

const LoadedDeck &VisualDeckStorageModel::deckForRow(int row) const
{
    return dataForRow(row).deck;
}

int VisualDeckStorageModel::rowForFilePath(const QString &filePath) const
{
    return rowByFilePath.value(filePath, -1);
}

/**
 * @brief Rebuilds the file path -> row index from scratch after bulk changes.
 */
void VisualDeckStorageModel::reindexFilePaths()
{
    rowByFilePath.clear();
    for (int row = 0; row < decks.size(); ++row) {
        const QString &filePath = decks.at(row).filePath;
        // First row wins, mirroring what a linear scan would return for duplicates.
        if (!rowByFilePath.contains(filePath)) {
            rowByFilePath.insert(filePath, row);
        }
    }
}

void VisualDeckStorageModel::startScan()
{
    scanCancelled->store(true, std::memory_order_relaxed);
    scanCancelled = std::make_shared<std::atomic_bool>(false);
    const auto cancelled = scanCancelled;
    const int generation = ++scanGeneration;
    const QString currentDeckPath = deckPath;
    const QPointer<VisualDeckStorageModel> guard(this);
    beginResetModel();
    if (!guard) {
        return;
    }
    decks.clear();
    rowByFilePath.clear();
    folderPaths.clear();
    pendingLoads.clear();
    endResetModel();
    if (!guard || generation != scanGeneration || currentDeckPath.isEmpty()) {
        return;
    }

    // The continuation remains bound to the model; the pool owns its worker tasks.
    auto scan = QtConcurrent::run(&taskPool, [currentDeckPath, cancelled] {
        if (cancelled->load(std::memory_order_relaxed)) {
            return DeckScanResult{};
        }
        return scanDeckDirectory(currentDeckPath, cancelled);
    });
    scan.then(this, [this, generation](const DeckScanResult &result) {
        if (generation != scanGeneration) {
            return;
        }
        const QPointer<VisualDeckStorageModel> guard(this);
        folderPaths = result.folderPaths;
        if (result.decks.isEmpty()) {
            return;
        }
        beginInsertRows(QModelIndex(), 0, result.decks.size() - 1);
        if (!guard) {
            return;
        }
        decks = result.decks;
        reindexFilePaths();
        endInsertRows();
        if (!guard || generation != scanGeneration) {
            return;
        }
        for (int row = 0; row < decks.size(); ++row) {
            beginLoad(row);
        }
    });
}

void VisualDeckStorageModel::beginLoad(int row)
{
    if (row < 0 || row >= decks.size() || decks.at(row).loadInProgress) {
        return;
    }

    DeckPreviewData &data = decks[row];
    data.loadInProgress = true;
    const quint64 loadId = data.loadId = ++nextLoadId;
    const auto cancelled = scanCancelled;

    const QString filePath = data.filePath;
    const DeckFileFormat::Format fmt = DeckFileFormat::getFormatFromName(filePath);
    const int generation = scanGeneration;

    auto load = QtConcurrent::run(&taskPool, [filePath, fmt, cancelled]() -> std::optional<DeckLoadResult> {
        if (cancelled->load(std::memory_order_relaxed)) {
            return std::nullopt;
        }
        std::optional<LoadedDeck> deck = DeckLoader::loadFromFile(filePath, fmt, false);
        if (!deck || cancelled->load(std::memory_order_relaxed)) {
            return std::nullopt;
        }
        // Color identity walks every card through the database, so compute it here to
        // keep the completion handler on the UI thread cheap.
        const QString colorIdentity = getDeckColorIdentity(deck->deckList, CardDatabaseManager::query());
        return DeckLoadResult{std::move(*deck), QFileInfo(filePath).lastModified(), colorIdentity};
    });

    // Bound to this model as context: runs on the UI thread and is dropped if the model is destroyed first.
    load.then(this, [this, filePath, generation, loadId](std::optional<DeckLoadResult> result) {
        if (generation != scanGeneration) {
            return; // The deck list was re-scanned while this load was running; drop the stale result.
        }

        // Queue the result and apply a bounded number per event loop turn so that
        // finishing hundreds of loads at once cannot stall the UI thread.
        PendingDeckLoad pending;
        pending.filePath = filePath;
        pending.generation = generation;
        pending.loadId = loadId;
        if (result) {
            pending.ok = true;
            pending.deck = std::move(result->deck);
            pending.lastModified = result->lastModified;
            pending.colorIdentity = std::move(result->colorIdentity);
        }
        pendingLoads.append(std::move(pending));
        schedulePendingLoadDrain();
    });
}

void VisualDeckStorageModel::schedulePendingLoadDrain()
{
    if (drainScheduled) {
        return;
    }
    drainScheduled = true;
    QMetaObject::invokeMethod(this, &VisualDeckStorageModel::drainPendingLoads, Qt::QueuedConnection);
}

void VisualDeckStorageModel::drainPendingLoads()
{
    drainScheduled = false;
    const QPointer<VisualDeckStorageModel> guard(this);

    QElapsedTimer timer;
    timer.start();

    while (!pendingLoads.isEmpty()) {
        PendingDeckLoad pending = pendingLoads.takeFirst();

        if (pending.generation != scanGeneration) {
            continue;
        }

        const int row = rowForFilePath(pending.filePath);
        if (row == -1) {
            continue;
        }

        DeckPreviewData &data = decks[row];
        if (pending.loadId != data.loadId) {
            continue;
        }
        data.loadInProgress = false;

        if (!pending.ok) {
            continue; // Leave the row unloaded so it stays visible without deck data.
        }

        data.deck = std::move(pending.deck);
        data.loadSucceeded = true;
        data.lastModified = pending.lastModified;
        recomputeDeckMetadata(data, false);
        data.colorIdentity = std::move(pending.colorIdentity);

        ++data.revision;
        emit dataChanged(index(row), index(row));
        if (!guard) {
            return;
        }
        // A direct signal receiver can refresh, rename or remove rows (or close
        // the widget). Do not announce the old row as a newly loaded deck.
        const int currentRow = rowForFilePath(pending.filePath);
        if (pending.generation == scanGeneration && currentRow >= 0 && decks.at(currentRow).loadId == pending.loadId) {
            emit deckLoaded(currentRow);
            if (!guard) {
                return;
            }
        }

        // Checked after applying at least one load, so a single slow application
        // still makes progress instead of starving the queue.
        if (timer.elapsed() >= LOAD_DRAIN_TIME_BUDGET_MS) {
            break;
        }
    }

    if (!pendingLoads.isEmpty()) {
        schedulePendingLoadDrain();
    }
}

/**
 * @brief Recomputes all derived metadata of a row from its loaded deck.
 */
void VisualDeckStorageModel::recomputeDeckMetadata(DeckPreviewData &data, bool recomputeColorIdentity)
{
    const DeckList &deckList = data.deck.deckList;

    data.deckName = deckList.getName();
    data.displayName = !data.deckName.isEmpty() ? data.deckName : QFileInfo(data.deck.lastLoadInfo.fileName).fileName();
    data.tags = deckList.getTags();
    data.lastLoaded = QDateTime::fromString(deckList.getLastLoadedTimestamp());
    data.bannerCard = deckList.getBannerCard();
    if (recomputeColorIdentity) {
        data.colorIdentity = getDeckColorIdentity(data.deck.deckList, CardDatabaseManager::query());
    }
}

void VisualDeckStorageModel::setFilePathForRow(int row, const QString &newFilePath)
{
    if (row < 0 || row >= decks.size()) {
        return;
    }

    DeckPreviewData &data = decks[row];
    rowByFilePath.remove(data.filePath);
    data.filePath = newFilePath;
    data.relativeFilePath = relativeFilePathFor(newFilePath, deckPath);
    data.folderPath = folderPathFor(newFilePath, deckPath);
    if (!rowByFilePath.contains(newFilePath)) {
        rowByFilePath.insert(newFilePath, row);
    }
    if (data.loadInProgress) {
        // The old job still identifies the previous path. Start a new request so
        // the renamed row completes, and reject even an old A -> B -> A result.
        data.loadInProgress = false;
        beginLoad(row);
    }
}

bool VisualDeckStorageModel::renameDeck(int row, const QString &newName)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    const QString oldName = data.deck.deckList.getName();
    data.deck.deckList.setName(newName);
    if (!DeckLoader::saveToFile(data.deck)) {
        data.deck.deckList.setName(oldName);
        return false;
    }

    recomputeDeckMetadata(data);
    ++data.revision;
    emit dataChanged(index(row), index(row), {VisualDeckStorageRoles::DisplayNameRole});
    return true;
}

bool VisualDeckStorageModel::renameFile(int row, const QString &newBaseName)
{
    if (row < 0 || row >= decks.size() || newBaseName.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    const QFileInfo info(data.filePath);
    if (newBaseName == info.baseName()) {
        return false;
    }

    QString newFileName = newBaseName;
    if (!info.suffix().isEmpty()) {
        newFileName += "." + info.suffix();
    }

    const QString newFilePath = QFileInfo(info.dir(), newFileName).filePath();
    if (!QFile::rename(info.filePath(), newFilePath)) {
        return false;
    }

    const QString oldFilePath = data.filePath;
    data.deck.lastLoadInfo.fileName = newFilePath;
    setFilePathForRow(row, newFilePath);
    data.lastModified = QFileInfo(newFilePath).lastModified();

    const QPointer<VisualDeckStorageModel> guard(this);
    const QPersistentModelIndex changedRow(index(row));
    ++data.revision;
    emit dataChanged(index(row), index(row));
    if (guard && changedRow.isValid() && dataForRow(changedRow.row()).filePath == newFilePath) {
        emit deckFilePathChanged(oldFilePath, newFilePath);
    }
    return true;
}

bool VisualDeckStorageModel::deleteFile(int row)
{
    if (row < 0 || row >= decks.size()) {
        return false;
    }

    const QString filePath = decks.at(row).filePath;
    if (!QFile::remove(QFileInfo(filePath).filePath())) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    rowByFilePath.remove(filePath);
    decks.removeAt(row);
    // Rows after the deleted one shift down by one.
    for (auto it = rowByFilePath.begin(); it != rowByFilePath.end(); ++it) {
        if (it.value() > row) {
            --it.value();
        }
    }
    endRemoveRows();
    return true;
}

bool VisualDeckStorageModel::setTags(int row, const QStringList &tags)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    const QStringList oldTags = data.deck.deckList.getTags();
    data.deck.deckList.setTags(tags);
    if (!DeckLoader::saveToFile(data.deck)) {
        data.deck.deckList.setTags(oldTags);
        return false;
    }

    data.tags = tags;
    ++data.revision;
    emit dataChanged(index(row), index(row), {VisualDeckStorageRoles::TagsRole});
    return true;
}

bool VisualDeckStorageModel::setBannerCard(int row, const CardRef &cardRef)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    const CardRef oldBannerCard = data.deck.deckList.getBannerCard();
    data.deck.deckList.setBannerCard(cardRef);
    if (!DeckLoader::saveToFile(data.deck)) {
        data.deck.deckList.setBannerCard(oldBannerCard);
        return false;
    }

    data.bannerCard = cardRef;
    ++data.revision;
    emit dataChanged(index(row), index(row),
                     {VisualDeckStorageRoles::BannerCardNameRole, VisualDeckStorageRoles::BannerCardProviderIdRole});
    return true;
}

bool VisualDeckStorageModel::convertToCockatriceFormat(int row)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    const QString oldFilePath = data.filePath;
    if (!DeckLoader::convertToCockatriceFormat(data.deck)) {
        return false;
    }

    setFilePathForRow(row, data.deck.lastLoadInfo.fileName);
    data.lastModified = QFileInfo(data.filePath).lastModified();
    recomputeDeckMetadata(data);

    // dataChanged can synchronously reset/remove rows or destroy the model.
    // Keep values, not a reference into decks, across signal delivery.
    const QString newFilePath = data.filePath;
    const QPointer<VisualDeckStorageModel> guard(this);
    const QPersistentModelIndex changedRow(index(row));
    ++data.revision;
    emit dataChanged(index(row), index(row));
    if (guard && changedRow.isValid() && oldFilePath != newFilePath &&
        dataForRow(changedRow.row()).filePath == newFilePath) {
        emit deckFilePathChanged(oldFilePath, newFilePath);
    }
    return true;
}

bool VisualDeckStorageModel::reloadIfModified(int row)
{
    if (row < 0 || row >= decks.size()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    QFileInfo fileInfo(data.filePath);
    const QDateTime newLastModified = fileInfo.lastModified();
    if (!newLastModified.isValid() || newLastModified <= data.lastModified) {
        return false;
    }

    std::optional<LoadedDeck> result =
        DeckLoader::loadFromFile(data.filePath, DeckFileFormat::getFormatFromName(data.filePath), false);
    if (!result) {
        return false;
    }

    data.deck = *result;
    data.loadId = ++nextLoadId;
    data.loadInProgress = false;
    data.loadSucceeded = true;
    data.lastModified = fileInfo.lastModified();
    recomputeDeckMetadata(data);

    const QPointer<VisualDeckStorageModel> guard(this);
    const QPersistentModelIndex loadedRow(index(row));
    const quint64 loadId = data.loadId;
    ++data.revision;
    emit dataChanged(index(row), index(row));
    if (guard && loadedRow.isValid() && dataForRow(loadedRow.row()).loadId == loadId) {
        emit deckLoaded(loadedRow.row());
    }
    return true;
}
