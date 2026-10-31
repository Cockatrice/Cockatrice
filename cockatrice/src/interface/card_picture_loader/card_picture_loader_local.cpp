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

    connect(folderWatcher, &QFileSystemWatcher::directoryChanged, indexRefreshTimer, qOverload<>(&QTimer::start));
    connect(folderWatcher, &QFileSystemWatcher::fileChanged, indexRefreshTimer, qOverload<>(&QTimer::start));

    fallbackRefreshTimer->setInterval(FALLBACK_REFRESH_INTERVAL_MS);
    connect(fallbackRefreshTimer, &QTimer::timeout, this, &CardPictureLoaderLocal::refreshIndex);
    fallbackRefreshTimer->start();

    refreshIndex();
}

void CardPictureLoaderLocal::refreshIndex()
{
    customFolderIndex.clear();
    QSet<QString> directories;

    QDir customDir(customPicsPath);
    if (customDir.exists()) {
        directories.insert(customDir.absolutePath());
    }

    QDirIterator it(customPicsPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    // Recursively check all subdirectories of the CUSTOM folder
    while (it.hasNext()) {
        const QString thisPath = it.next();
        const QFileInfo thisFileInfo = it.fileInfo();

        if (thisFileInfo.isDir()) {
            directories.insert(thisFileInfo.absoluteFilePath());
        } else if (thisFileInfo.isFile()) {
            // We don't know which name is the correctedName because there might be '.'s in the cardName.
            // Just add all possibilities to be sure.
            customFolderIndex.insert(thisFileInfo.baseName(), thisFileInfo.absoluteFilePath());
            customFolderIndex.insert(thisFileInfo.completeBaseName(), thisFileInfo.absoluteFilePath());
        }
    }

    updateWatchedDirectories(directories);

    qCDebug(CardPictureLoaderLocalLog) << "Finished indexing local image folder CUSTOM; map now has"
                                       << customFolderIndex.size() << "entries.";
    emit indexChanged();
}

void CardPictureLoaderLocal::updateWatchedDirectories(const QSet<QString> &directories)
{
    const QSet<QString> stale = watchedDirectories - directories;
    if (!stale.isEmpty()) {
        folderWatcher->removePaths(QStringList(stale.begin(), stale.end()));
    }

    const QSet<QString> added = directories - watchedDirectories;
    if (!added.isEmpty()) {
        const QStringList failed = folderWatcher->addPaths(QStringList(added.begin(), added.end()));
        if (!failed.isEmpty()) {
            qCDebug(CardPictureLoaderLocalLog) << "Could not watch custom picture directories:" << failed;
        }
    }

    watchedDirectories = directories;
}

QStringList CardPictureLoaderLocal::indexedPaths(const QString &baseName) const
{
    return customFolderIndex.values(baseName);
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

        QStringList candidatePaths = customFolderIndex.values(nameVariant);

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
    picsPath = SettingsCache::instance().paths().getPicsPath();
    customPicsPath = SettingsCache::instance().paths().getCustomPicsPath();
    refreshIndex();
}
