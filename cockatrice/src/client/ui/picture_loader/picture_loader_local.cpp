#include "picture_loader_local.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../settings/cache_settings.h"
#include "picture_to_load.h"

#include <QDirIterator>
#include <QMovie>

static constexpr int REFRESH_INTERVAL_MS = 10 * 1000;

PictureLoaderLocal::PictureLoaderLocal(QObject *parent)
    : QObject(parent), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath()),
      overrideAllCardArtWithPersonalPreference(SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference())
{
    // Hook up signals to settings
    connect(&SettingsCache::instance(), &SettingsCache::picsPathChanged, this, &PictureLoaderLocal::picsPathChanged);
    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            &PictureLoaderLocal::setOverrideAllCardArtWithPersonalPreference);

    refreshIndex();

    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &PictureLoaderLocal::refreshIndex);
    refreshTimer->start(REFRESH_INTERVAL_MS);
}

void PictureLoaderLocal::refreshIndex()
{
    customFolderIndex.clear();

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

    qCDebug(PictureLoaderLocalLog) << "Finished indexing local image folder CUSTOM; map now has"
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

    CardInfoPerSet setInstance = CardDatabaseManager::getInstance()->getSetInfoForCard(toLoad);

    QString cardName = toLoad->getName();
    QString correctedCardName = toLoad->getCorrectedName();

    QString setName, collectorNumber, providerId;

    if (setInstance.getPtr()) {
        setName = setInstance.getPtr()->getCorrectedShortName();
        collectorNumber = setInstance.getProperty("num");
        providerId = setInstance.getProperty("uuid");
    }

    qCDebug(PictureLoaderLocalLog).nospace()
        << "[card: " << cardName << " set: " << setName << "]: Attempting to load picture from local";
    return tryLoadCardImageFromDisk(setName, correctedCardName, collectorNumber, providerId);
}

QImage PictureLoaderLocal::tryLoadCardImageFromDisk(const QString &setName,
                                                    const QString &correctedCardName,
                                                    const QString &collectorNumber,
                                                    const QString &providerId) const
{
    QImage image;
    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);

    // Order is cardName_providerId, cardName_setName_collectorNumber, setName-collectorNumber-cardName and then just
    // generically cardName, if the user has decided to override every printing. Most-to-least specific.

    const QStringList nameVariants = {!providerId.isEmpty() ? QString("%1_%2").arg(correctedCardName, providerId)
                                                            : QString(),
                                      (!setName.isEmpty() && !collectorNumber.isEmpty())
                                          ? QString("%1_%2_%3").arg(correctedCardName, setName, collectorNumber)
                                          : QString(),
                                      (!setName.isEmpty() && !collectorNumber.isEmpty())
                                          ? QString("%1-%2-%3").arg(setName, collectorNumber, correctedCardName)
                                          : QString(),
                                      correctedCardName};

    for (const QString &nameVariant : nameVariants) {
        if (nameVariant.isEmpty()) {
            continue;
        }

        QStringList candidatePaths;
        const auto matches = customFolderIndex.values(nameVariant);

        for (const QString &path : matches) {
            candidatePaths << path;
        }

        if (!setName.isEmpty()) {
            candidatePaths << picsPath + "/" + setName + "/" + nameVariant;
            candidatePaths << picsPath + "/downloadedPics/" + setName + "/" + nameVariant;
        }

        for (const QString &path : candidatePaths) {
            for (const QString suffix : {"", ".full", ".xlhq"}) {
                QString fullPath = path + suffix;
                QFileInfo fi(fullPath);
                if (!fi.exists() || !fi.isFile()) {
                    continue;
                }

                imgReader.setFileName(fullPath);
                if (imgReader.read(&image)) {
                    qCDebug(PictureLoaderLocalLog).nospace()
                        << "[card: " << correctedCardName << " set: " << setName << "] Found picture at: " << fullPath;
                    return image;
                }
            }
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
