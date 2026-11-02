#ifndef PICTURE_LOADER_LOCAL_H
#define PICTURE_LOADER_LOCAL_H

#include <QDateTime>
#include <QHash>
#include <QImage>
#include <QPair>
#include <QLoggingCategory>
#include <QMultiHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <qtmetamacros.h>

class ExactCard;
class QFileSystemWatcher;
class QTimer;

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
     * @brief Constructs the loader against explicit folders without SettingsCache.
     *
     * Used by tests and by callers that already resolved the configured paths.
     */
    CardPictureLoaderLocal(QObject *parent, const QString &picsPath, const QString &customPicsPath);

    /**
     * @brief Returns every indexed file whose base name matches @p baseName.
     */
    [[nodiscard]] QStringList indexedPaths(const QString &baseName) const;

    /**
     * @brief Attempts to load a card image from local disk or custom folders.
     * @param toLoad ExactCard object representing the card to load.
     * @return Loaded QImage if found; otherwise, an empty QImage.
     *
     * Uses a set of name variants and folder paths to attempt to locate the correct image.
     */
    [[nodiscard]] QImage tryLoad(const ExactCard &toLoad) const;

    /** Whether any local import name for this exact printing is in the changed set. */
    [[nodiscard]] static bool matchesNames(const ExactCard &card, const QSet<QString> &names);
    [[nodiscard]] static bool mayMatchSharedKey(const ExactCard &card, const QSet<QString> &names);

private:
    QString picsPath;       ///< Path to standard card image folder
    QString customPicsPath; ///< Path to custom card image folder

    QMultiHash<QString, QString> customFolderIndex; ///< Multimap from cardName to file paths in CUSTOM folder
    QHash<QString, int> customFileOrder; ///< QDir filename ordering for ambiguous same-directory names
    QFileSystemWatcher *folderWatcher;              ///< Watches the custom folder tree for changes
    QTimer *indexRefreshTimer;                      ///< Debounces bursts of filesystem events
    QTimer *fallbackRefreshTimer;                   ///< Slow poll for filesystems without change notification
    mutable QHash<QString, QPair<QDateTime, qint64>> watchedImages; ///< Attempted custom files and last observed state
    static constexpr int MAX_FILE_WATCHES = 128;
    mutable QStringList recentImages; ///< Most recently accessed custom files, newest first
    mutable QSet<QString> failedFileWatches; ///< Retry failed registrations on the periodic fallback
    mutable QSet<QString> pendingNames; ///< File changes collected during a debounce window
    bool pendingAll = false; ///< Configured root changed, so every card needs reevaluation

    /**
     * @brief Rebuilds the index of the custom image folder.
     *
     * Iterates through all subdirectories of the CUSTOM folder and populates
     * `customFolderIndex` with all discovered image files keyed by base name
     * without the final file extension.
     */
    void refreshIndex();

    /**
     * @brief Keeps the watcher registered on exactly @p directories.
     */
    void updateWatchedDirectories(const QSet<QString> &directories);

    /** Keep at most MAX_FILE_WATCHES native file watches; retain metadata for all attempted files. */
    void rememberImage(const QString &path) const;
    void reconcileFileWatches() const;

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

signals:
    /**
     * @brief Emitted after the custom folder index has been rebuilt.
     */
    void indexChanged();

    /** Changed exact basenames, or all cards when configured roots changed. */
    void imagesChanged(const QStringList &names, bool all);

private slots:
    /**
     * @brief Updates internal paths when the user changes picture settings.
     *
     * Triggered by `SettingsCache::picsPathChanged`.
     */
    void picsPathChanged();
};

#endif // PICTURE_LOADER_LOCAL_H
