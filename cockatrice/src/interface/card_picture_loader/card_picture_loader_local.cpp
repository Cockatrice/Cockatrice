#include "card_picture_loader_local.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_loader_local_schemes.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QImageReader>
#include <QList>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QStringList>
#include <QTimer>
#include <libcockatrice/settings/paths_settings.h>

#include <algorithm>
#include <limits>

static constexpr int REFRESH_DEBOUNCE_MS = 500;
// Watching can fail on network and FUSE mounts; the slow poll still picks those
// changes up without the old ten-second full rescan.
static constexpr int FALLBACK_REFRESH_INTERVAL_MS = 5 * 60 * 1000;

CardPictureLoaderLocal::CardPictureLoaderLocal(QObject *parent)
    : CardPictureLoaderLocal(parent,
                             SettingsCache::instance().paths().getPicsPath(),
                             SettingsCache::instance().paths().getCustomPicsPath())
{
    // Hook up signals to settings
    connect(&SettingsCache::instance().paths(), &PathsSettings::picsPathChanged, this,
            &CardPictureLoaderLocal::picsPathChanged);
}

CardPictureLoaderLocal::CardPictureLoaderLocal(QObject *parent,
                                               const QString &_picsPath,
                                               const QString &_customPicsPath)
    : QObject(parent), picsPath(_picsPath), customPicsPath(_customPicsPath),
      folderWatcher(new QFileSystemWatcher(this)), indexRefreshTimer(new QTimer(this)),
      fallbackRefreshTimer(new QTimer(this))
{
    indexRefreshTimer->setSingleShot(true);
    indexRefreshTimer->setInterval(REFRESH_DEBOUNCE_MS);
    connect(indexRefreshTimer, &QTimer::timeout, this, &CardPictureLoaderLocal::refreshIndex);

    connect(folderWatcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        indexRefreshTimer->start();
    });
    connect(folderWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
        pendingNames.insert(QFileInfo(path).completeBaseName());
        indexRefreshTimer->start();
    });

    fallbackRefreshTimer->setInterval(FALLBACK_REFRESH_INTERVAL_MS);
    connect(fallbackRefreshTimer, &QTimer::timeout, this, [this] {
        // Failed native registrations are retried at the slow fallback cadence,
        // not on every filesystem event or repeated card lookup.
        failedFileWatches.clear();
        refreshIndex();
    });
    fallbackRefreshTimer->start();

    refreshIndex();
}

void CardPictureLoaderLocal::refreshIndex()
{
    const auto previousIndex = customFolderIndex;
    customFolderIndex.clear();
    QSet<QString> directories;
    QSet<QString> indexedFiles;
    QSet<QString> nameAndDirectory;
    QSet<QString> ambiguousDirectories;
    customFileOrder.clear();

    QDir customDir(customPicsPath);
    if (customDir.exists()) {
        directories.insert(customDir.absolutePath());
    } else {
        // Watch the closest existing parent so a newly created custom folder
        // is indexed promptly, including when intermediate folders are absent.
        QString parentPath = QFileInfo(customDir.absolutePath()).absolutePath();
        while (!QDir(parentPath).exists()) {
            const QString nextParent = QFileInfo(parentPath).absolutePath();
            if (nextParent == parentPath) {
                break;
            }
            parentPath = nextParent;
        }
        if (QDir(parentPath).exists()) {
            directories.insert(parentPath);
        }
    }

    QDirIterator it(customPicsPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    // Recursively check all subdirectories of the CUSTOM folder
    while (it.hasNext()) {
        const QString thisPath = it.next();
        const QFileInfo thisFileInfo = it.fileInfo();

        if (thisFileInfo.isDir()) {
            directories.insert(thisFileInfo.absoluteFilePath());
        } else if (thisFileInfo.isFile()) {
            // completeBaseName removes only the image extension, preserving
            // periods that are part of the card name.
            const QString path = thisFileInfo.absoluteFilePath();
            customFolderIndex.insert(thisFileInfo.completeBaseName(), path);
            const QString nameInDirectory = thisFileInfo.absolutePath() + QChar::Null + thisFileInfo.completeBaseName();
            if (nameAndDirectory.contains(nameInDirectory)) {
                ambiguousDirectories.insert(thisFileInfo.absolutePath());
            }
            nameAndDirectory.insert(nameInDirectory);
            indexedFiles.insert(path);
        }
    }

    // The original loader resolved files within each directory using QDir's
    // default filename ordering. Cache that order only for directories where
    // multiple files share a basename, keeping repeated lookups scan-free.
    for (const QString &directory : ambiguousDirectories) {
        const QDir dir(directory);
        const QStringList files = dir.entryList(QDir::Files);
        for (int i = 0; i < files.size(); ++i) {
            customFileOrder.insert(dir.absoluteFilePath(files.at(i)), i);
        }
    }

    updateWatchedDirectories(directories);

    QSet<QString> changedNames = pendingNames;
    pendingNames.clear();
    const auto oldNames = previousIndex.uniqueKeys();
    const auto newNames = customFolderIndex.uniqueKeys();
    QSet<QString> allNames(oldNames.cbegin(), oldNames.cend());
    allNames.unite(QSet<QString>(newNames.cbegin(), newNames.cend()));
    for (const QString &name : allNames) {
        // Preserve ordering changes as well as additions, removals and renames.
        if (previousIndex.values(name) != customFolderIndex.values(name)) {
            changedNames.insert(name);
        }
    }
    const bool all = pendingAll;
    pendingAll = false;
    // Watch only custom images attempted for a card. If a file watch was
    // lost after replacement or failed transiently, retry it on refresh. The
    // slow fallback also compares file metadata when watching is unavailable.
    for (auto it = watchedImages.begin(); it != watchedImages.end();) {
        if (!indexedFiles.contains(it.key())) {
            recentImages.removeAll(it.key());
            failedFileWatches.remove(it.key());
            it = watchedImages.erase(it);
            continue;
        }
        const QFileInfo fileInfo(it.key());
        const auto state = qMakePair(fileInfo.lastModified(), fileInfo.size());
        if (state != it.value()) {
            changedNames.insert(fileInfo.completeBaseName());
            it.value() = state;
        }
        ++it;
    }
    reconcileFileWatches();

    qCDebug(CardPictureLoaderLocalLog) << "Finished indexing local image folder CUSTOM; map now has"
                                       << customFolderIndex.size() << "entries.";
    if (all || !changedNames.isEmpty()) {
        emit indexChanged();
        emit imagesChanged(QStringList(changedNames.cbegin(), changedNames.cend()), all);
    }
}

void CardPictureLoaderLocal::rememberImage(const QString &path) const
{
    recentImages.removeAll(path);
    recentImages.prepend(path);
    while (recentImages.size() > MAX_FILE_WATCHES) {
        recentImages.removeLast();
    }
    reconcileFileWatches();
}

void CardPictureLoaderLocal::reconcileFileWatches() const
{
    const QSet<QString> desired(recentImages.cbegin(), recentImages.cend());
    const QStringList watched = folderWatcher->files();
    const QSet<QString> installed(watched.cbegin(), watched.cend());
    const QSet<QString> stale = installed - desired;
    if (!stale.isEmpty()) {
        folderWatcher->removePaths(QStringList(stale.cbegin(), stale.cend()));
    }
    const QStringList remaining = folderWatcher->files();
    const QSet<QString> retained(remaining.cbegin(), remaining.cend());
    int available = qMax(0, MAX_FILE_WATCHES - static_cast<int>(remaining.size()));
    // Iterate in LRU order so limited OS resources go to the newest files.
    for (const QString &path : recentImages) {
        if (available > 0 && !retained.contains(path) && !failedFileWatches.contains(path)) {
            if (folderWatcher->addPath(path)) {
                --available;
            } else {
                failedFileWatches.insert(path);
            }
        }
    }
}

void CardPictureLoaderLocal::updateWatchedDirectories(const QSet<QString> &directories)
{
    // QFileSystemWatcher may drop a watch when a directory is removed. Its
    // live list, rather than the previous desired set, is the source of truth.
    const QStringList watched = folderWatcher->directories();
    const QSet<QString> current(watched.cbegin(), watched.cend());
    const QSet<QString> stale = current - directories;
    if (!stale.isEmpty()) {
        folderWatcher->removePaths(QStringList(stale.begin(), stale.end()));
    }

    const QSet<QString> added = directories - current;
    if (!added.isEmpty()) {
        const QStringList failed = folderWatcher->addPaths(QStringList(added.begin(), added.end()));
        if (!failed.isEmpty()) {
            qCDebug(CardPictureLoaderLocalLog) << "Could not watch custom picture directories:" << failed;
        }
    }
}

QStringList CardPictureLoaderLocal::indexedPaths(const QString &baseName) const
{
    return customFolderIndex.values(baseName);
}

bool CardPictureLoaderLocal::matchesNames(const ExactCard &card, const QSet<QString> &names)
{
    const PrintingInfo printing = card.getPrinting();
    QString setName, collectorNumber, providerId;
    if (printing.getSet()) {
        setName = printing.getSet()->getCorrectedShortName();
        collectorNumber = printing.getProperty("num");
        providerId = printing.getUuid();
    }
    const auto variants = CardPictureLoaderLocalSchemes::generateImportVariants(
        card.getInfo().getCorrectedName(), setName, collectorNumber, providerId);
    for (const QString &variant : variants) {
        if (names.contains(variant)) {
            return true;
        }
    }
    return false;
}

bool CardPictureLoaderLocal::mayMatchSharedKey(const ExactCard &card, const QSet<QString> &names)
{
    if (matchesNames(card, names)) {
        return true;
    }
    // Cache keys may conflate different printings of the same card when UUIDs
    // are absent. Another observer can therefore match a different import
    // variant even though the active request's printing does not.
    const QString name = card.getInfo().getCorrectedName();
    if (name.isEmpty()) {
        return false;
    }
    for (const QString &changed : names) {
        if (changed.startsWith(name + QLatin1Char('_')) || changed.endsWith(QLatin1Char('_') + name)) {
            return true;
        }
    }
    return false;
}

QImage CardPictureLoaderLocal::tryLoad(const ExactCard &toLoad) const
{
    PrintingInfo setInstance = toLoad.getPrinting();

    QString cardName = toLoad.getName();
    QString correctedCardName = toLoad.getInfo().getCorrectedName();

    QString setName, collectorNumber, providerId;

    if (setInstance.getSet()) {
        setName = setInstance.getSet()->getCorrectedShortName();
        collectorNumber = setInstance.getProperty("num");
        providerId = setInstance.getUuid();
    }

    qCDebug(CardPictureLoaderLocalLog).nospace()
        << "[card: " << cardName << " set: " << setName << "]: Attempting to load picture from local";
    return tryLoadCardImageFromDisk(setName, correctedCardName, collectorNumber, providerId);
}

QImage CardPictureLoaderLocal::tryLoadCardImageFromDisk(const QString &setName,
                                                        const QString &correctedCardName,
                                                        const QString &collectorNumber,
                                                        const QString &providerId) const
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);

    // Most-to-least specific, these will fall through in order.
    QStringList nameVariants =
        CardPictureLoaderLocalSchemes::generateImportVariants(correctedCardName, setName, collectorNumber, providerId);

    for (const QString &nameVariant : nameVariants) {
        if (nameVariant.isEmpty()) {
            continue;
        }

        // Indexed custom entries are already exact file paths. Opening them
        // directly avoids scanning their entire directory for every card.
        const QStringList customPaths = customFolderIndex.values(nameVariant);
        QStringList orderedPaths;
        QSet<QString> visitedDirectories;
        for (const QString &candidate : customPaths) {
            const QString directory = QFileInfo(candidate).absolutePath();
            if (visitedDirectories.contains(directory)) {
                continue;
            }
            visitedDirectories.insert(directory);
            QStringList inDirectory;
            for (const QString &path : customPaths) {
                if (QFileInfo(path).absolutePath() == directory) {
                    inDirectory.append(path);
                }
            }
            std::stable_sort(inDirectory.begin(), inDirectory.end(), [this](const QString &a, const QString &b) {
                return customFileOrder.value(a, std::numeric_limits<int>::max()) <
                       customFileOrder.value(b, std::numeric_limits<int>::max());
            });
            orderedPaths.append(inDirectory);
        }
        for (const QString &path : orderedPaths) {
            // Watch before decoding so a concurrent edit cannot fall between
            // the read and registration. Compare metadata as a fallback for a
            // change in that interval or an unavailable file watch.
            const QFileInfo before(path);
            const auto stateBefore = qMakePair(before.lastModified(), before.size());
            rememberImage(path);
            // Failed decodes also need watching: an invalid custom image may
            // later be repaired in place without changing the directory.
            watchedImages.insert(path, stateBefore);
            imgReader.setFileName(path);
            const bool decoded = imgReader.read(&image);
            const QFileInfo after(path);
            if (qMakePair(after.lastModified(), after.size()) != stateBefore) {
                pendingNames.insert(before.completeBaseName());
                indexRefreshTimer->start();
            }
            if (decoded) {
                qCDebug(CardPictureLoaderLocalLog).nospace()
                    << "[card: " << correctedCardName << " set: " << setName << "] Found picture at: " << path;
                return image;
            }
        }

        QStringList candidatePaths;
        if (!setName.isEmpty()) {
            candidatePaths << picsPath + "/" + setName + "/" + nameVariant;
            candidatePaths << picsPath + "/downloadedPics/" + setName + "/" + nameVariant;
        }

        // Non-set-folder export schemes (e.g., Name_Set_Collector) write straight into
        // downloadedPics/; check there as a fallback so local overrides round-trip.
        candidatePaths << picsPath + "/downloadedPics/" + nameVariant;

        for (const QString &path : candidatePaths) {
            QFileInfo fileInfo(path);
            QDir dir = fileInfo.dir();
            QString baseName = fileInfo.completeBaseName();

            if (!dir.exists()) {
                continue;
            }

            QStringList files = dir.entryList(QDir::Files);
            for (const QString &file : files) {
                QFileInfo fi(file);
                if (fi.completeBaseName() != baseName) {
                    continue;
                }

                QString fullPath = dir.filePath(file);
                imgReader.setFileName(fullPath);

                if (imgReader.read(&image)) {
                    qCDebug(CardPictureLoaderLocalLog).nospace()
                        << "[card: " << correctedCardName << " set: " << setName << "] Found picture at: " << fullPath;
                    return image;
                }
            }
        }
    }

    qCDebug(CardPictureLoaderLocalLog).nospace()
        << "[card: " << correctedCardName << " set: " << setName << "]: Picture NOT found on disk.";
    return QImage();
}

void CardPictureLoaderLocal::picsPathChanged()
{
    pendingAll = true;
    picsPath = SettingsCache::instance().paths().getPicsPath();
    customPicsPath = SettingsCache::instance().paths().getCustomPicsPath();
    refreshIndex();
}
