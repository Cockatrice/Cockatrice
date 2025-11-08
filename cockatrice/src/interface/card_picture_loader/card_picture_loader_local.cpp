#include "card_picture_loader_local.h"

#include "../../client/settings/cache_settings.h"
#include "card_picture_to_load.h"

#include <QDirIterator>
#include <QMovie>
#include <libcockatrice/card/database/card_database_manager.h>

static constexpr int REFRESH_INTERVAL_MS = 10 * 1000;

CardPictureLoaderLocal::CardPictureLoaderLocal(QObject *parent)
    : QObject(parent), picsPath(SettingsCache::instance().getPicsPath()),
      customPicsPath(SettingsCache::instance().getCustomPicsPath())
{
    // Hook up signals to settings
    connect(&SettingsCache::instance(), &SettingsCache::picsPathChanged, this,
            &CardPictureLoaderLocal::picsPathChanged);

    refreshIndex();

    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &CardPictureLoaderLocal::refreshIndex);
    refreshTimer->start(REFRESH_INTERVAL_MS);
}

void CardPictureLoaderLocal::refreshIndex()
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

    qCDebug(CardPictureLoaderLocalLog) << "Finished indexing local image folder CUSTOM; map now has"
                                       << customFolderIndex.size() << "entries.";
}

/**
 * Tries to load the card image from the local images.
 *
 * @param toLoad The card to load
 * @return The loaded image, or an empty QImage if loading failed.
 */
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
    QStringList nameVariants;

    // cardName_providerId
    if (!providerId.isEmpty()) {
        nameVariants << QString("%1-%2").arg(correctedCardName, providerId)
                     << QString("%1_%2").arg(correctedCardName, providerId);
    }
    // cardName_setName_collectorNumber & setName-collectorNumber-cardName
    if (!setName.isEmpty() && !collectorNumber.isEmpty()) {
        nameVariants << QString("%1_%2_%3").arg(correctedCardName, setName, collectorNumber)
                     << QString("%1-%2-%3").arg(setName, collectorNumber, correctedCardName);
    }
    // cardName_setName
    if (!setName.isEmpty()) {
        nameVariants << QString("%1_%2").arg(correctedCardName, setName)
                     << QString("%1-%2").arg(setName, correctedCardName);
    }

    // cardName
    nameVariants << correctedCardName;

    for (const QString &nameVariant : nameVariants) {
        if (nameVariant.isEmpty()) {
            continue;
        }

        QStringList candidatePaths = customFolderIndex.values(nameVariant);

        if (!setName.isEmpty()) {
            candidatePaths << picsPath + "/" + setName + "/" + nameVariant;
            candidatePaths << picsPath + "/downloadedPics/" + setName + "/" + nameVariant;
        }

        for (const QString &path : candidatePaths) {
            QFileInfo fileInfo(path);
            QDir dir = fileInfo.dir();
            QString baseName = fileInfo.fileName();

            if (!dir.exists()) {
                continue;
            }

            QStringList files = dir.entryList(QDir::Files);
            for (const QString &file : files) {
                if (!file.startsWith(baseName)) {
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
    picsPath = SettingsCache::instance().getPicsPath();
    customPicsPath = SettingsCache::instance().getCustomPicsPath();
}
