#ifndef CARD_PICTURE_LOADER_H
#define CARD_PICTURE_LOADER_H

#include "card_picture_loader_status_bar.h"
#include "card_picture_loader_worker.h"

#include <QLoggingCategory>
#include <libcockatrice/card/card_info.h>

inline Q_LOGGING_CATEGORY(CardPictureLoaderLog, "card_picture_loader");
inline Q_LOGGING_CATEGORY(CardPictureLoaderCardBackCacheFailLog, "card_picture_loader.card_back_cache_fail");

/**
 * @class CardPictureLoader
 * @ingroup PictureLoader
 * @brief Singleton class to manage card image loading and caching. Provides functionality to asynchronously load,
 * cache, and manage card images for the client.
 *
 * This class is a singleton and handles:
 * - Loading card images from disk or network.
 * - Caching images in QPixmapCache for fast reuse.
 * - Providing themed card backs, including fallback and in-progress/failed states.
 * - Emitting updates when pixmaps are loaded.
 *
 * It interacts with CardPictureLoaderWorker for background loading and
 * CardPictureLoaderStatusBar to display loading progress in the main window.
 *
 * Provides static accessors for:
 * - Card images by ExactCard.
 * - Card back images (normal, in-progress, failed).
 * - Cache management (clearPixmapCache(), clearNetworkCache(), cacheCardPixmaps(const QList<ExactCard> &cards)).
 *
 * Uses a worker thread for asynchronous image loading and a status bar widget
 * to track load progress.
 */
class CardPictureLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Access the singleton instance of CardPictureLoader.
     * @return Reference to the singleton.
     */
    static CardPictureLoader &getInstance()
    {
        static CardPictureLoader instance;
        return instance;
    }

private:
    explicit CardPictureLoader();
    ~CardPictureLoader() override;

    // Disable copy and assignment for singleton
    CardPictureLoader(CardPictureLoader const &) = delete;
    void operator=(CardPictureLoader const &) = delete;

    CardPictureLoaderWorker *worker;       ///< Worker thread for async image loading
    CardPictureLoaderStatusBar *statusBar; ///< Status bar widget showing load progress

public:
    /**
     * @brief Retrieve a card pixmap, either from cache or enqueued for loading.
     * @param pixmap Reference to QPixmap where result will be stored.
     * @param card ExactCard to load.
     * @param size Desired size of pixmap.
     */
    static void getPixmap(QPixmap &pixmap, const ExactCard &card, QSize size);

    /**
     * @brief Retrieve a generic card back pixmap.
     * @param pixmap Reference to QPixmap where result will be stored.
     * @param size Desired size of pixmap.
     */
    static void getCardBackPixmap(QPixmap &pixmap, QSize size);

    /**
     * @brief Retrieve a card back pixmap for the loading-in-progress state.
     * @param pixmap Reference to QPixmap where result will be stored.
     * @param size Desired size of pixmap.
     */
    static void getCardBackLoadingInProgressPixmap(QPixmap &pixmap, QSize size);

    /**
     * @brief Retrieve a card back pixmap for the loading-failed state.
     * @param pixmap Reference to QPixmap where result will be stored.
     * @param size Desired size of pixmap.
     */
    static void getCardBackLoadingFailedPixmap(QPixmap &pixmap, QSize size);

    /**
     * @brief Preload a list of cards into the pixmap cache (limited to CACHED_CARD_PER_DECK_MAX).
     * @param cards List of ExactCard objects to preload.
     */
    static void cacheCardPixmaps(const QList<ExactCard> &cards);

    /**
     * @brief Check if the user has custom card art in the picsPath directory.
     * @return True if any custom art exists.
     */
    static bool hasCustomArt();

    /**
     * @brief Clears the in-memory QPixmap cache for all cards.
     */
    static void clearPixmapCache();

public slots:
    /**
     * @brief Clears the network disk cache of the worker.
     */
    static void clearNetworkCache();

    /**
     * @brief Slot called by the worker when an image is loaded.
     * Inserts the pixmap into the cache and emits pixmap updated signals.
     * @param card ExactCard that was loaded.
     * @param image Loaded QImage.
     */
    void imageLoaded(const ExactCard &card, const QImage &image);

private slots:
    /**
     * @brief Triggered when the user changes the picture download settings.
     * Clears the QPixmap cache to reload images.
     */
    void picDownloadChanged();

    /**
     * @brief Triggered when the pictures path changes.
     * Clears the QPixmap cache to reload images.
     */
    void picsPathChanged();
};

#endif
