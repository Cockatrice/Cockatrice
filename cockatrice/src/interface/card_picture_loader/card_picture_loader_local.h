#ifndef PICTURE_LOADER_LOCAL_H
#define PICTURE_LOADER_LOCAL_H

#include <QTimer>
#include <libcockatrice/card/printing/exact_card.h>

inline Q_LOGGING_CATEGORY(CardPictureLoaderLocalLog, "card_picture_loader.local");

/**
 * @class CardPictureLoaderLocal
 * @ingroup PictureLoader
 * @brief Handles searching for and loading card images from local and custom image folders.
 *
 * This class maintains an index of the CUSTOM folder to avoid repeatedly scanning
 * directories, and supports periodic refreshes to update the index.
 *
 * Responsibilities:
 * - Load images for ExactCard objects from local disk or custom folders.
 * - Maintain an index for fast lookup of images in the CUSTOM folder.
 * - Refresh the index periodically to account for changes in local image directories.
 */
class CardPictureLoaderLocal : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a CardPictureLoaderLocal object.
     * @param parent Optional parent QObject.
     *
     * Initializes paths from SettingsCache, connects to settings change signals,
     * builds the initial folder index, and starts a periodic refresh timer.
     */
    explicit CardPictureLoaderLocal(QObject *parent);

    /**
     * @brief Attempts to load a card image from local disk or custom folders.
     * @param toLoad ExactCard object representing the card to load.
     * @return Loaded QImage if found; otherwise, an empty QImage.
     *
     * Uses a set of name variants and folder paths to attempt to locate the correct image.
     */
    [[nodiscard]] QImage tryLoad(const ExactCard &toLoad) const;

private:
    QString picsPath;       ///< Path to standard card image folder
    QString customPicsPath; ///< Path to custom card image folder

    QMultiHash<QString, QString> customFolderIndex; ///< Multimap from cardName to file paths in CUSTOM folder
    QTimer *refreshTimer;                           ///< Timer for periodic folder index refresh

    /**
     * @brief Rebuilds the index of the custom image folder.
     *
     * Iterates through all subdirectories of the CUSTOM folder and populates
     * `customFolderIndex` with all discovered image files keyed by base name
     * and complete base name.
     */
    void refreshIndex();

    /**
     * @brief Attempts to load a card image from disk given its set and name info.
     * @param setName Corrected short name of the card's set.
     * @param correctedCardName Corrected card name (e.g., normalized name).
     * @param collectorNumber Collector number of the card.
     * @param providerId Optional provider UUID of the card.
     * @return Loaded QImage if found; otherwise, an empty QImage.
     *
     * Searches in both the custom folder index and standard pictures paths.
     * Uses several filename patterns to match card images, in order from
     * most-specific to least-specific.
     */
    [[nodiscard]] QImage tryLoadCardImageFromDisk(const QString &setName,
                                                  const QString &correctedCardName,
                                                  const QString &collectorNumber,
                                                  const QString &providerId) const;

private slots:
    /**
     * @brief Updates internal paths when the user changes picture settings.
     *
     * Triggered by `SettingsCache::picsPathChanged`.
     */
    void picsPathChanged();
};

#endif // PICTURE_LOADER_LOCAL_H
