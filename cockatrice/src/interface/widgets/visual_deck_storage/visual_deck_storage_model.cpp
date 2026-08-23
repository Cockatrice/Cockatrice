#include "visual_deck_storage_model.h"

#include "../../deck_loader/deck_loader.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QSet>
#include <QtConcurrentRun>
#include <libcockatrice/card/database/card_database_manager.h>
#include <utility>

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
 * @brief How many finished background deck loads one event loop turn applies.
 *
 * Applying a load emits dataChanged and wakes up the proxies, which is not
 * free, and the bound keeps a burst of simultaneous completions from stalling
 * the UI thread.
 */
constexpr int DECK_LOADS_PER_TURN = 24;

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
DeckScanResult scanDeckDirectory(const QString &deckPath)
{
    DeckScanResult result;

    QDirIterator fileIt(deckPath, DeckLoader::ACCEPTED_FILE_EXTENSIONS, QDir::Files,
                        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (fileIt.hasNext()) {
        const QString filePath = fileIt.next();
        DeckPreviewData data;
        data.filePath = filePath;
        data.relativeFilePath = relativeFilePathFor(filePath, deckPath);
        data.folderPath = folderPathFor(filePath, deckPath);
        data.lastModified = QFileInfo(filePath).lastModified();
        result.decks.append(std::move(data));
    }

    QSet<QString> seenFolders;
    QDirIterator folderIt(deckPath, QDir::Dirs | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (folderIt.hasNext()) {
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

static QString computeColorIdentity(const LoadedDeck &deck);

VisualDeckStorageModel::VisualDeckStorageModel(QObject *parent) : QAbstractListModel(parent)
{
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
    for (int i = 0; i < decks.size(); ++i) {
        if (decks.at(i).filePath == filePath) {
            return i;
        }
    }
    return -1;
}

void VisualDeckStorageModel::startScan()
{
    ++scanGeneration;
    beginResetModel();
    decks.clear();
    folderPaths.clear();
    pendingLoads.clear();
    endResetModel();

    if (deckPath.isEmpty()) {
        return;
    }

    const QString currentDeckPath = deckPath;
    const int generation = scanGeneration;

    // The scan (directory walk + one stat per file) runs on a worker thread so that
    // constructing the widget never stalls the UI thread on a large deck folder.
    auto *watcher = new QFutureWatcher<DeckScanResult>(this);
    connect(watcher, &QFutureWatcher<DeckScanResult>::finished, this, [this, watcher, generation] {
        watcher->deleteLater();

        if (generation != scanGeneration) {
            return; // A newer scan started while this one was running; drop the stale result.
        }

        const DeckScanResult result = watcher->result();
        folderPaths = result.folderPaths;

        if (result.decks.isEmpty()) {
            return;
        }

        beginInsertRows(QModelIndex(), 0, result.decks.size() - 1);
        decks = result.decks;
        endInsertRows();

        for (int row = 0; row < decks.size(); ++row) {
            beginLoad(row);
        }
    });

    watcher->setFuture(
        QtConcurrent::run([currentDeckPath]() -> DeckScanResult { return scanDeckDirectory(currentDeckPath); }));
}

void VisualDeckStorageModel::beginLoad(int row)
{
    if (row < 0 || row >= decks.size() || decks.at(row).loadInProgress) {
        return;
    }

    DeckPreviewData &data = decks[row];
    data.loadInProgress = true;

    const QString filePath = data.filePath;
    const DeckFileFormat::Format fmt = DeckFileFormat::getFormatFromName(filePath);
    const int generation = scanGeneration;

    auto *watcher = new QFutureWatcher<std::optional<DeckLoadResult>>(this);
    connect(watcher, &QFutureWatcher<std::optional<DeckLoadResult>>::finished, this,
            [this, watcher, filePath, generation] {
                watcher->deleteLater();

                if (generation != scanGeneration) {
                    return; // The deck list was re-scanned while this load was running; drop the stale result.
                }

                // Queue the result and apply a bounded number per event loop turn so that
                // finishing hundreds of loads at once cannot stall the UI thread.
                const std::optional<DeckLoadResult> result = watcher->result();
                PendingDeckLoad pending;
                pending.filePath = filePath;
                pending.generation = generation;
                if (result) {
                    pending.ok = true;
                    pending.deck = std::move(result->deck);
                    pending.lastModified = result->lastModified;
                    pending.colorIdentity = std::move(result->colorIdentity);
                }
                pendingLoads.append(std::move(pending));
                schedulePendingLoadDrain();
            });

    watcher->setFuture(QtConcurrent::run([filePath, fmt]() -> std::optional<DeckLoadResult> {
        std::optional<LoadedDeck> deck = DeckLoader::loadFromFile(filePath, fmt, false);
        if (!deck) {
            return std::nullopt;
        }
        // Color identity walks every card through the database, so compute it here to
        // keep the completion handler on the UI thread cheap.
        const QString colorIdentity = computeColorIdentity(*deck);
        return DeckLoadResult{std::move(*deck), QFileInfo(filePath).lastModified(), colorIdentity};
    }));
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

    int applied = 0;
    while (!pendingLoads.isEmpty() && applied < DECK_LOADS_PER_TURN) {
        PendingDeckLoad pending = pendingLoads.takeFirst();

        if (pending.generation != scanGeneration) {
            continue;
        }

        const int row = rowForFilePath(pending.filePath);
        if (row == -1) {
            continue;
        }

        DeckPreviewData &data = decks[row];
        data.loadInProgress = false;

        if (!pending.ok) {
            ++applied;
            continue; // Leave the row unloaded so it stays visible without deck data.
        }

        data.deck = std::move(pending.deck);
        data.loadSucceeded = true;
        data.lastModified = pending.lastModified;
        recomputeDeckMetadata(data, false);
        data.colorIdentity = std::move(pending.colorIdentity);

        emit dataChanged(index(row), index(row));
        emit deckLoaded(row);
        ++applied;
    }

    if (!pendingLoads.isEmpty()) {
        schedulePendingLoadDrain();
    }
}

/**
 * @brief Computes the color identity of a deck in WUBRG order.
 */
static QString computeColorIdentity(const LoadedDeck &deck)
{
    QStringList cardList = deck.deckList.getCardList({DECK_ZONE_MAIN, DECK_ZONE_SIDE});
    if (cardList.isEmpty()) {
        return {};
    }

    QSet<QChar> colorSet; // A set to collect unique color symbols (e.g., W, U, B, R, G)

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = CardDatabaseManager::query()->getCardInfo(cardName);
        if (currentCard) {
            const QString colors = currentCard->getColors(); // Something like "WUB"
            for (const QChar &color : colors) {
                colorSet.insert(color);
            }
        }
    }

    // Ensure the color identity is in WUBRG order
    QString colorIdentity;
    const QString wubrgOrder = "WUBRG";
    for (const QChar &color : wubrgOrder) {
        if (colorSet.contains(color)) {
            colorIdentity.append(color);
        }
    }

    return colorIdentity;
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
        data.colorIdentity = computeColorIdentity(data.deck);
    }
}

void VisualDeckStorageModel::setFilePathForRow(int row, const QString &newFilePath)
{
    if (row < 0 || row >= decks.size()) {
        return;
    }

    DeckPreviewData &data = decks[row];
    data.filePath = newFilePath;
    data.relativeFilePath = relativeFilePathFor(newFilePath, deckPath);
    data.folderPath = folderPathFor(newFilePath, deckPath);
}

bool VisualDeckStorageModel::renameDeck(int row, const QString &newName)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    data.deck.deckList.setName(newName);
    if (!DeckLoader::saveToFile(data.deck)) {
        return false;
    }

    recomputeDeckMetadata(data);
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

    emit dataChanged(index(row), index(row));
    emit deckFilePathChanged(oldFilePath, newFilePath);
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
    decks.removeAt(row);
    endRemoveRows();
    return true;
}

bool VisualDeckStorageModel::setTags(int row, const QStringList &tags)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    data.deck.deckList.setTags(tags);
    if (!DeckLoader::saveToFile(data.deck)) {
        return false;
    }

    data.tags = tags;
    emit dataChanged(index(row), index(row), {VisualDeckStorageRoles::TagsRole});
    return true;
}

bool VisualDeckStorageModel::setBannerCard(int row, const CardRef &cardRef)
{
    if (row < 0 || row >= decks.size() || decks.at(row).deck.isEmpty()) {
        return false;
    }

    DeckPreviewData &data = decks[row];
    data.deck.deckList.setBannerCard(cardRef);
    if (!DeckLoader::saveToFile(data.deck)) {
        return false;
    }

    data.bannerCard = cardRef;
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

    emit dataChanged(index(row), index(row));
    if (oldFilePath != data.filePath) {
        emit deckFilePathChanged(oldFilePath, data.filePath);
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
    data.loadSucceeded = true;
    data.lastModified = fileInfo.lastModified();
    recomputeDeckMetadata(data);

    emit dataChanged(index(row), index(row));
    emit deckLoaded(row);
    return true;
}
