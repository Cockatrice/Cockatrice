#include "card_picture_loader.h"

#include "../../client/settings/cache_settings.h"
#include "../pixel_map_generator.h"
#include "card_picture_loader_cache_method.h"
#include "card_picture_loader_local_schemes.h"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QMetaObject>
#include <QMovie>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmapCache>
#include <QScreen>
#include <QStatusBar>
#include <QThread>
#include <algorithm>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <utility>

// never cache more than 300 cards at once for a single deck
#define CACHED_CARD_PER_DECK_MAX 300

// wait at least this long before retrying a card whose picture failed to load
static constexpr int RETRY_FAILED_CARDS_SECS = 300;

CardPictureLoader::CardPictureLoader() : QObject(nullptr)
{
    worker = new CardPictureLoaderWorker;
    connect(&SettingsCache::instance().paths(), &PathsSettings::picsPathChanged, this,
            &CardPictureLoader::picsPathChanged);
    connect(&SettingsCache::instance().downloads(), &DownloadSettings::picDownloadChanged, this,
            &CardPictureLoader::picDownloadChanged);
    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::cardLangChanged, this,
            &CardPictureLoader::cardLangChanged);

    qRegisterMetaType<ExactCard>("ExactCard");
    connect(worker, &CardPictureLoaderWorker::imageLoaded, this, &CardPictureLoader::imageLoaded);

    statusBar = new CardPictureLoaderStatusBar(nullptr);
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(QApplication::activeWindow());
    if (mainWindow) {
        mainWindow->statusBar()->addPermanentWidget(statusBar);
    }

    connect(worker, &CardPictureLoaderWorker::imageRequestQueued, statusBar,
            &CardPictureLoaderStatusBar::addQueuedImageLoad);
    connect(worker, &CardPictureLoaderWorker::imageRequestSucceeded, statusBar,
            &CardPictureLoaderStatusBar::addSuccessfulImageLoad);
}

CardPictureLoader::~CardPictureLoader()
{
    if (worker) {
        // Capture the thread first: shutdownThread() blocks until the worker has been freed by the
        // finished() -> deleteLater chain, after which the worker pointer must not be dereferenced.
        QThread *pictureLoaderThread = worker->workerThread();
        worker->shutdownThread();
        worker = nullptr;
        delete pictureLoaderThread;
    }
}

void CardPictureLoader::getCardBackPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + "x" + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(CardPictureLoaderLog) << "PictureLoader: cache miss for" << backCacheKey;
        QPixmap tmpPixmap = themePixmap(QStringLiteral("cardback"));

        if (tmpPixmap.isNull()) {
            qCWarning(CardPictureLoaderLog) << "Failed to load 'theme:cardback'! Using fallback pixmap.";
            tmpPixmap = QPixmap(size);
            tmpPixmap.fill(Qt::gray); // Fallback to a gray pixmap
        } else {
            qCDebug(CardPictureLoaderLog) << "Successfully loaded 'theme:cardback'.";
        }

        pixmap = tmpPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void CardPictureLoader::getCardBackLoadingInProgressPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey =
        "_trice_card_back_inprogress_" + QString::number(size.width()) + "x" + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(CardPictureLoaderCardBackCacheFailLog) << "PictureLoader: cache miss for" << backCacheKey;
        QPixmap tmpPixmap = themePixmap(QStringLiteral("cardback"));

        if (tmpPixmap.isNull()) {
            qCWarning(CardPictureLoaderLog) << "Failed to load 'theme:cardback' for in-progress state! Using fallback.";
            tmpPixmap = QPixmap(size);
            tmpPixmap.fill(Qt::blue); // Fallback with blue color
        } else {
            qCDebug(CardPictureLoaderCardBackCacheFailLog)
                << "Successfully loaded 'theme:cardback' for in-progress state.";
        }

        pixmap = tmpPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void CardPictureLoader::getCardBackLoadingFailedPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey =
        "_trice_card_back_failed_" + QString::number(size.width()) + "x" + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(CardPictureLoaderCardBackCacheFailLog) << "PictureLoader: cache miss for" << backCacheKey;
        QPixmap tmpPixmap = themePixmap(QStringLiteral("cardback"));

        if (tmpPixmap.isNull()) {
            qCWarning(CardPictureLoaderLog) << "Failed to load 'theme:cardback' for failed state! Using fallback.";
            tmpPixmap = QPixmap(size);
            tmpPixmap.fill(Qt::red); // Fallback with red color
        } else {
            qCDebug(CardPictureLoaderCardBackCacheFailLog) << "Successfully loaded 'theme:cardback' for failed state.";
        }

        pixmap = tmpPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void CardPictureLoader::getPixmap(QPixmap &pixmap, const ExactCard &card, QSize size)
{
    if (!card) {
        qCWarning(CardPictureLoaderLog) << "getPixmap called with null card!";
        return;
    }

    QString key = card.getPixmapCacheKey();
    QString sizeKey = key + QLatin1Char('_') + QString::number(size.width()) + "x" + QString::number(size.height());

    if (QPixmapCache::find(sizeKey, &pixmap)) {
        return; // Use cached version
    }

    // load the image and create a copy of the correct size
    QPixmap bigPixmap;
    if (QPixmapCache::find(key, &bigPixmap)) {
        if (bigPixmap.isNull()) {
            // Leave the pixmap null so callers fall back to a solid color
            // instead of showing the card back.
            QDateTime failedAtTime = getInstance().failedAt.value(key);
            if (!failedAtTime.isValid() ||
                failedAtTime.addSecs(RETRY_FAILED_CARDS_SECS) < QDateTime::currentDateTime()) {
                getInstance().failedAt.remove(key);
                QPixmapCache::remove(key);
                getInstance().worker->enqueueImageLoad(card);
            }
            return;
        }

        QScreen *screen = qApp->primaryScreen();
        qreal dpr = screen ? screen->devicePixelRatio() : 1.0;
        qCDebug(CardPictureLoaderLog) << "Scaling cached image for" << card.getName();

        pixmap = bigPixmap.scaled(size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(dpr);
        QPixmapCache::insert(sizeKey, pixmap);
        return;
    }

    // add the card to the load queue
    qCDebug(CardPictureLoaderLog) << "Enqueuing " << card.getName() << " for " << card.getPixmapCacheKey();
    getInstance().worker->enqueueImageLoad(card);
}

void CardPictureLoader::imageLoaded(const ExactCard &card, const QImage &image)
{
    QPixmap finalPixmap;

    if (image.isNull()) {
        getInstance().failedAt.insert(card.getPixmapCacheKey(), QDateTime::currentDateTime());
        qCDebug(CardPictureLoaderLog) << "Caching NULL pixmap for" << card.getName();
    } else {
        getInstance().failedAt.remove(card.getPixmapCacheKey());
        if (card.getInfo().getUiAttributes().upsideDownArt) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))
            QImage mirrorImage = image.flipped(Qt::Horizontal | Qt::Vertical);
#else
            QImage mirrorImage = image.mirrored(true, true);
#endif
            finalPixmap = QPixmap::fromImage(mirrorImage);
        } else {
            finalPixmap = QPixmap::fromImage(image);
        }
    }

    QPixmapCache::insert(card.getPixmapCacheKey(), finalPixmap);

    if (static_cast<CardPictureLoaderCacheMethod::CacheMethod>(
            SettingsCache::instance().cacheStorage().getCardPictureLoaderCacheMethod()) ==
        CardPictureLoaderCacheMethod::CacheMethod::FILESYSTEM_CACHE) {
        saveCardImageToLocalStorage(card, finalPixmap);
    }

    // imageLoaded should only be reached if the exactCard isn't already in cache.
    // (plus there's a deduplication mechanism in CardPictureLoaderWorker)
    // It should be safe to connect the CardInfo here without worrying about redundant connections.
    connect(card.getCardPtr().data(), &QObject::destroyed, this, [cacheKey = card.getPixmapCacheKey()] {
        QPixmapCache::remove(cacheKey);
        getInstance().failedAt.remove(cacheKey);
    });

    card.emitPixmapUpdated();
}

void CardPictureLoader::deleteAllLocalOverrides(const ExactCard &card)
{
    const QString picsRoot = SettingsCache::instance().paths().getPicsPath();
    if (picsRoot.isEmpty() || !card) {
        return;
    }

    QDir baseDir(picsRoot);
    if (!baseDir.cd("downloadedPics")) {
        return;
    }

    const QString name = card.getInfo().getCorrectedName();

    QString set, collector, uuid;
    auto printing = card.getPrinting();
    if (printing.getSet()) {
        set = printing.getSet()->getCorrectedShortName();
        collector = printing.getProperty("num");
        uuid = printing.getUuid();
    }

    for (const auto &scheme : CardPictureLoaderLocalSchemes::exportSchemes()) {
        QString rel = CardPictureLoaderLocalSchemes::expandPattern(scheme.pattern, name, set, collector, uuid);

        if (rel.isEmpty()) {
            continue;
        }

        rel += ".png";
        rel = QDir::cleanPath(rel);

        QString fullPath = baseDir.filePath(rel);

        if (QFile::exists(fullPath)) {
            QFile::remove(fullPath);
        }
    }
}

void CardPictureLoader::saveCardImageToLocalStorage(const ExactCard &card,
                                                    const QPixmap &pixmap,
                                                    const bool allowOverwrite)
{
    if (pixmap.isNull() || !card) {
        return;
    }

    const QString picsRoot = SettingsCache::instance().paths().getPicsPath();
    CardPictureLoaderLocalSchemes::NamingScheme scheme = static_cast<CardPictureLoaderLocalSchemes::NamingScheme>(
        SettingsCache::instance().cacheStorage().getLocalCardImageStorageNamingScheme());

    QString pattern;

    for (const auto &s : CardPictureLoaderLocalSchemes::exportSchemes()) {
        if (s.id == scheme) {
            pattern = s.pattern;
            break;
        }
    }

    if (picsRoot.isEmpty() || pattern.isEmpty()) {
        return;
    }

    // Base directory: <picsPath>/downloadedPics
    QDir baseDir(picsRoot);
    if (!baseDir.exists("downloadedPics")) {
        baseDir.mkpath("downloadedPics");
    }
    baseDir.cd("downloadedPics");

    // Collect card metadata
    const QString cardName = card.getInfo().getCorrectedName();

    QString setName;
    QString collectorNumber;
    QString uuid;

    PrintingInfo printing = card.getPrinting();
    if (printing.getSet()) {
        setName = printing.getSet()->getCorrectedShortName();
        collectorNumber = printing.getProperty("num");
        uuid = printing.getUuid();
    }

    // Build path from scheme
    QString relativePath =
        CardPictureLoaderLocalSchemes::expandPattern(pattern, cardName, setName, collectorNumber, uuid);

    if (relativePath.isEmpty()) {
        return;
    }

    // append extension
    relativePath += ".png";

    // Normalize slashes
    relativePath = QDir::cleanPath(relativePath);

    QFileInfo outInfo(baseDir.filePath(relativePath));

    // Automatic cache writes (FILESYSTEM_CACHE) must never clobber an explicit user override.
    // Only the explicit override paths pass allowOverwrite == true.
    if (!allowOverwrite && outInfo.exists()) {
        return;
    }

    QDir outDir = outInfo.dir();

    // Ensure directory exists
    if (!outDir.exists()) {
        if (!baseDir.mkpath(outDir.path())) {
            qCWarning(CardPictureLoaderLog) << "Failed to create directory for downloaded card image:" << outDir.path();
            return;
        }
    }

    // Save image
    QImage image = pixmap.toImage();
    if (!image.save(outInfo.absoluteFilePath(), "PNG")) {
        qCWarning(CardPictureLoaderLog) << "Failed to save card image to" << outInfo.absoluteFilePath();
    }
}

void CardPictureLoader::installPrintingOverrideOnLoad(const ExactCard &originalCard, const ExactCard &overrideCard)
{
    // Overriding a card with itself is the reset case, not a real override: every code path below
    // would re-enter itself through emitPixmapUpdated(). Reject it outright.
    if (originalCard == overrideCard) {
        return;
    }

    CardInfoPtr cardPtr = overrideCard.getCardPtr();
    if (!cardPtr) {
        return;
    }

    // Heap-allocate so the lambda can capture it before the connection is made
    auto *connectionHandle = new QMetaObject::Connection;

    *connectionHandle =
        connect(cardPtr.data(), &CardInfo::pixmapUpdated, cardPtr.data(),
                [originalCard, overrideCard, connectionHandle, this](const PrintingInfo &printing) {
                    // All printings share the same CardInfo, so ignore updates triggered by any
                    // other printing (e.g., the original card re-loading from disk).
                    if (printing != overrideCard.getPrinting()) {
                        return;
                    }

                    QPixmap pixmap;
                    if (QPixmapCache::find(overrideCard.getPixmapCacheKey(), &pixmap) && !pixmap.isNull()) {
                        // The override art has resolved — persist it and reflect it immediately.
                        // Retire the connection before emitting so the refresh can't re-enter.
                        saveCardImageToLocalStorage(originalCard, pixmap, /*allowOverwrite=*/true);

                        QObject::disconnect(*connectionHandle);
                        delete connectionHandle;

                        QPixmapCache::clear();
                        originalCard.emitPixmapUpdated();
                        return;
                    }

                    // The art could not be resolved. Keep the connection armed so a late resolution
                    // still lands, and surface a visible refusal instead of a silent no-op. An
                    // override already on disk is left untouched and simply re-displayed.
                    QPixmapCache::clear();
                    if (!hasLocalOverrides(originalCard)) {
                        QPixmap refusedPixmap;
                        getCardBackLoadingFailedPixmap(refusedPixmap, QSize(480, 672));
                        QPixmapCache::insert(originalCard.getPixmapCacheKey(), refusedPixmap);
                    }
                    originalCard.emitPixmapUpdated();
                });

    // Now enqueue; if the image is already loading (deduplicated in the worker),
    // the signal will still fire when it completes
    CardPictureLoader::getInstance().worker->enqueueImageLoad(overrideCard);
}

void CardPictureLoader::installPrintingOverride(const ExactCard &originalCard, const ExactCard &overrideCard)
{
    // Same guard as installPrintingOverrideOnLoad: self-override is the reset case.
    if (originalCard == overrideCard) {
        return;
    }

    QPixmap pixmap;
    const QString key = overrideCard.getPixmapCacheKey();

    if (QPixmapCache::find(key, &pixmap) && !pixmap.isNull()) {
        // Already cached — save immediately; the caller refreshes the card.
        saveCardImageToLocalStorage(originalCard, pixmap, /*allowOverwrite=*/true);
        return;
    }

    // Cache miss or previously failed load — enqueue load and wait for the signal.
    installPrintingOverrideOnLoad(originalCard, overrideCard);
}

bool CardPictureLoader::hasLocalOverrides(const ExactCard &card)
{
    const QString picsRoot = SettingsCache::instance().paths().getPicsPath();
    if (picsRoot.isEmpty() || !card) {
        return false;
    }

    QDir baseDir(picsRoot);
    if (!baseDir.cd("downloadedPics")) {
        return false;
    }

    const QString name = card.getInfo().getCorrectedName();

    QString set, collector, uuid;
    const PrintingInfo printing = card.getPrinting();
    if (printing.getSet()) {
        set = printing.getSet()->getCorrectedShortName();
        collector = printing.getProperty("num");
        uuid = printing.getUuid();
    }

    for (const auto &scheme : CardPictureLoaderLocalSchemes::exportSchemes()) {
        QString rel = CardPictureLoaderLocalSchemes::expandPattern(scheme.pattern, name, set, collector, uuid);

        if (rel.isEmpty()) {
            continue;
        }

        rel += ".png";
        rel = QDir::cleanPath(rel);

        if (QFile::exists(baseDir.filePath(rel))) {
            return true;
        }
    }

    return false;
}

void CardPictureLoader::clearPixmapCache()
{
    QPixmapCache::clear();
}

void CardPictureLoader::clearNetworkCache()
{
    auto &worker = *getInstance().worker;
    // The disk cache and redirect cache are owned by the worker thread; clearing them from the
    // UI thread would race with the worker's cache reads/writes. Block until the worker thread
    // has executed the clear so the "Cached card pictures have been reset." message is truthful.
    if (worker.isRunning()) {
        QMetaObject::invokeMethod(&worker, "clearNetworkCache", Qt::BlockingQueuedConnection);
    } else {
        worker.clearNetworkCache();
    }
}

void CardPictureLoader::cacheCardPixmaps(const QList<ExactCard> &cards)
{
    QPixmap tmp;
    int max = qMin(cards.size(), CACHED_CARD_PER_DECK_MAX);
    for (int i = 0; i < max; ++i) {
        const ExactCard &card = cards.at(i);
        if (!card) {
            continue;
        }

        QString key = card.getPixmapCacheKey();
        if (QPixmapCache::find(key, &tmp)) {
            continue;
        }

        getInstance().worker->enqueueImageLoad(card);
    }
}

void CardPictureLoader::picDownloadChanged()
{
    QPixmapCache::clear();
}

void CardPictureLoader::picsPathChanged()
{
    QPixmapCache::clear();
}

void CardPictureLoader::cardLangChanged()
{
    // Localized images are fetched via a different URL, but the in-memory
    // pixmap cache is keyed by card name/uuid, so drop everything cached
    // (including failure timestamps) to force a reload in the new language.
    QPixmapCache::clear();
    failedAt.clear();
}
