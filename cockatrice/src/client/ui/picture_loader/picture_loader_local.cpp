#include "picture_loader_local.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_to_load.h"

#include <QDirIterator>
#include <QMovie>

PictureLoaderLocal::PictureLoaderLocal(QObject *parent)
    : QObject(parent), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath()),
      overrideAllCardArtWithPersonalPreference(SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference())
{
    // Hook up signals to settings
    connect(&SettingsCache::instance(), &SettingsCache::picsPathChanged, this, &PictureLoaderLocal::picsPathChanged);
    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            &PictureLoaderLocal::setOverrideAllCardArtWithPersonalPreference);

    createIndex();
}

void PictureLoaderLocal::createIndex()
{
    QDirIterator it(customPicsPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    // Recursively check all subdirectories of the CUSTOM folder
    while (it.hasNext()) {
        QString thisPath(it.next());
        QFileInfo thisFileInfo(thisPath);

        if (thisFileInfo.isFile()) {
            // We don't know which name is the correctedName because there might be '.'s in the cardName.
            // Just add all possibilities to be sure.
            customFolderIndex.insert(thisFileInfo.baseName(), thisFileInfo.absoluteFilePath());
            customFolderIndex.insert(thisFileInfo.completeBaseName(), thisFileInfo.absoluteFilePath());
        }
    }

    qCInfo(PictureLoaderLocalLog) << "Finished indexing local image folder CUSTOM; map now has"
                                  << customFolderIndex.size() << "entries.";
}

/**
 * Tries to load the card image from the local images.
 *
 * @param toLoad The card to load
 * @return The loaded image, or an empty QImage if loading failed.
 */
QImage PictureLoaderLocal::tryLoad(const CardInfoPtr &toLoad) const
{
    CardSetPtr set = PictureToLoad::extractSetsSorted(toLoad).first();

    QString setName = set ? set->getCorrectedShortName() : QString();
    QString cardName = toLoad->getName();
    QString correctedCardName = toLoad->getCorrectedName();

    // FIXME: This is a hack so that to keep old Cockatrice behavior
    // (ignoring provider ID) when the "override all card art with personal
    // preference" is set.
    //
    // Figure out a proper way to integrate the two systems at some point.
    //
    // Note: need to go through a member for
    // overrideAllCardArtWithPersonalPreference as reading from the
    // SettingsCache instance from the PictureLoaderWorker thread could
    // cause race conditions.
    //
    // XXX: Reading from the CardDatabaseManager instance from the
    // PictureLoaderWorker thread might not be safe either
    bool searchCustomPics =
        overrideAllCardArtWithPersonalPreference ||
        CardDatabaseManager::getInstance()->isProviderIdForPreferredPrinting(cardName, toLoad->getPixmapCacheKey());
    if (searchCustomPics) {
        qCDebug(PictureLoaderLocalLog).nospace()
            << "[card: " << cardName << " set: " << setName << "]: Attempting to load picture from local";
        return tryLoadCardImageFromDisk(setName, correctedCardName, searchCustomPics);
    }

    qCDebug(PictureLoaderLocalLog).nospace()
        << "[card: " << cardName << " set: " << setName << "]: Skipping load picture from local";

    return QImage();
}

QImage PictureLoaderLocal::tryLoadCardImageFromDisk(const QString &setName,
                                                    const QString &correctedCardName,
                                                    const bool searchCustomPics) const
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    QList<QString> picsPaths = QList<QString>();

    if (searchCustomPics) {
        picsPaths << customFolderIndex.values(correctedCardName);
    }

    if (!setName.isEmpty()) {
        picsPaths << picsPath + "/" + setName + "/" + correctedCardName
                  // We no longer store downloaded images there, but don't just ignore
                  // stuff that old versions have put there.
                  << picsPath + "/downloadedPics/" + setName + "/" + correctedCardName;
    }

    // Iterates through the list of paths, searching for images with the desired
    // name with any QImageReader-supported extension
    for (const auto &_picsPath : picsPaths) {
        if (!QFileInfo(_picsPath).isFile()) {
            continue;
        }

        imgReader.setFileName(_picsPath);
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderLocalLog).nospace()
                << "[card: " << correctedCardName << " set: " << setName << "]: Picture found on disk.";
            return image;
        }
        imgReader.setFileName(_picsPath + ".full");
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderLocalLog).nospace()
                << "[card: " << correctedCardName << " set: " << setName << "]: Picture.full found on disk.";
            return image;
        }
        imgReader.setFileName(_picsPath + ".xlhq");
        if (imgReader.read(&image)) {
            qCDebug(PictureLoaderLocalLog).nospace()
                << "[card: " << correctedCardName << " set: " << setName << "]: Picture.xlhq found on disk.";
            return image;
        }
    }
    qCDebug(PictureLoaderLocalLog).nospace()
        << "[card: " << correctedCardName << " set: " << setName << "]: Picture NOT found on disk.";
    return QImage();
}

void PictureLoaderLocal::picsPathChanged()
{
    picsPath = SettingsCache::instance().getPicsPath();
    customPicsPath = SettingsCache::instance().getCustomPicsPath();
}

void PictureLoaderLocal::setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArtWithPersonalPreference)
{
    overrideAllCardArtWithPersonalPreference = _overrideAllCardArtWithPersonalPreference;
}
