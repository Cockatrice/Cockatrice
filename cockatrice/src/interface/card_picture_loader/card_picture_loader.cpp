#include "card_picture_loader.h"

#include "../../client/settings/cache_settings.h"
#include "../pixel_map_generator.h"
#include "card_picture_loader_cache_method.h"
#include "card_picture_loader_local_schemes.h"
#include "card_picture_loader_local.h"
#include "card_picture_loader_status_bar.h"
#include "card_picture_loader_worker.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"

#include <QApplication>
#include <QChar>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QLatin1Char>
#include <QList>
#include <QMainWindow>
#include <QMessageLogger>
#include <QMetaObject>
#include <QMetaType>
#include <QPixmap>
#include <QPixmapCache>
#include <QScreen>
#include <QSharedPointer>
#include <QSize>
#include <QStatusBar>
#include <QString>
#include <QStringLiteral>
#include <QThread>
#include <QWidget>
#include <QtGlobal>
#include <compare>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <qnamespace.h>

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
    connect(worker, &CardPictureLoaderWorker::localImagesChanged, this, &CardPictureLoader::localImagesChanged);
    connect(worker, &CardPictureLoaderWorker::networkCacheCleared, this, &CardPictureLoader::networkCacheCleared);

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
        const bool stopped = worker->shutdownThread();
        worker = nullptr;
        // Deleting a QThread that is still running is undefined behaviour, so only free it once the
        // bounded wait in shutdownThread() confirmed that it stopped.
        if (stopped) {
            delete pictureLoaderThread;
        }
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

void CardPictureLoader::trackCard(const ExactCard &card, const QString &scaledKey)
{
    const QString key = card.getPixmapCacheKey();
    auto &entries = trackedCards[key];
    for (auto it = entries.begin(); it != entries.end();) {
        const auto existing = it->card.toStrongRef();
        if (!existing) {
            it = entries.erase(it);
            continue;
        }
        if (existing == card.getCardPtr() && it->printing == card.getPrinting()) {
            if (!scaledKey.isEmpty()) {
                it->scaledKeys.insert(scaledKey);
            }
            return;
        }
        ++it;
    }
    TrackedCard entry{card.getCardPtr().toWeakRef(), card.getPrinting(), {}};
    if (!scaledKey.isEmpty()) {
        entry.scaledKeys.insert(scaledKey);
    }
    entries.append(entry);
    // Do not keep card objects alive just because their pictures were shown.
    connect(card.getCardPtr().data(), &QObject::destroyed, this, [this, key] {
        auto it = trackedCards.find(key);
        if (it == trackedCards.end()) {
            return;
        }
        auto &cards = it.value();
        for (auto cardIt = cards.begin(); cardIt != cards.end();) {
            if (cardIt->card.isNull()) {
                for (const QString &scaled : cardIt->scaledKeys) {
                    QPixmapCache::remove(scaled);
                }
                cardIt = cards.erase(cardIt);
            } else {
                ++cardIt;
            }
        }
        if (cards.isEmpty()) {
            trackedCards.erase(it);
            QPixmapCache::remove(key);
            failedAt.remove(key);
        }
    });
}

void CardPictureLoader::localImagesChanged(const QStringList &names, bool all, const QStringList &fulfilledKeys)
{
    const QSet<QString> fulfilled(fulfilledKeys.cbegin(), fulfilledKeys.cend());
    const QSet<QString> changedNames(names.cbegin(), names.cend());
    // Select whole cache keys, so distinct live observers sharing a key are
    // invalidated and notified together even when only one printing matched.
    const auto snapshot = trackedCards;
    QSet<QString> affectedKeys;
    for (auto it = snapshot.cbegin(); it != snapshot.cend(); ++it) {
        for (const TrackedCard &entry : it.value()) {
            if (const auto card = entry.card.toStrongRef()) {
                if (all || CardPictureLoaderLocal::matchesNames(ExactCard(card, entry.printing), changedNames)) {
                    affectedKeys.insert(it.key());
                    break;
                }
            }
        }
    }
    for (const QString &key : affectedKeys) {
        QPixmapCache::remove(key);
        failedAt.remove(key);
        for (const TrackedCard &entry : snapshot.value(key)) {
            for (const QString &scaled : entry.scaledKeys) {
                QPixmapCache::remove(scaled);
            }
        }
        auto tracked = trackedCards.find(key);
        if (tracked != trackedCards.end()) {
            for (TrackedCard &entry : tracked.value()) {
                entry.scaledKeys.clear();
            }
        }
        if (fulfilled.contains(key)) {
            pendingLoads.insert(key);
            continue; // The worker will deliver this key immediately after invalidation.
        }
        for (const TrackedCard &entry : snapshot.value(key)) {
            if (const auto card = entry.card.toStrongRef()) {
                const ExactCard exact(card, entry.printing);
                if (all || CardPictureLoaderLocal::matchesNames(exact, changedNames)) {
                    // Select one matching observer for this shared key.
                    // Completion, not invalidation, drives the override flow.
                    pendingLoads.insert(key);
                    worker->refreshImageLoad(exact);
                    break;
                }
            }
        }
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
    getInstance().trackCard(card, sizeKey);

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
                if (!getInstance().pendingLoads.contains(key)) {
                    getInstance().pendingLoads.insert(key);
                    getInstance().worker->enqueueImageLoad(card);
                }
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
    if (!getInstance().pendingLoads.contains(key)) {
        qCDebug(CardPictureLoaderLog) << "Enqueuing " << card.getName() << " for " << key;
        getInstance().pendingLoads.insert(key);
        getInstance().worker->enqueueImageLoad(card);
    }
}

void CardPictureLoader::imageLoaded(const ExactCard &card, const QImage &image)
{
    pendingLoads.remove(card.getPixmapCacheKey());
    trackCard(card);
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

    // Several live CardInfo objects can share one pixmap key. The worker
    // deduplicates their request, so notify every tracked observer when that
    // shared image arrives, without retaining any card object.
    const auto observers = trackedCards.value(card.getPixmapCacheKey());
    for (const TrackedCard &entry : observers) {
        if (const auto observed = entry.card.toStrongRef()) {
            ExactCard(observed, entry.printing).emitPixmapUpdated();
        }
    }
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
    CardPictureLoader::getInstance().pendingLoads.insert(overrideCard.getPixmapCacheKey());
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
    // During teardown the worker is released before this singleton, so a queued clear may still
    // arrive with no worker left to run it.
    CardPictureLoaderWorker *worker = getInstance().worker;
    if (!worker) {
        return;
    }
    // The disk cache and redirect cache are owned by the worker thread, so the clear has to run
    // there. Invoke it asynchronously to keep the GUI responsive while the worker may be walking
    // the user's picture directories or recursively deleting the cache directory; callers that
    // need to know when it is done can listen for networkCacheCleared().
    QMetaObject::invokeMethod(worker, &CardPictureLoaderWorker::clearNetworkCache, Qt::QueuedConnection);
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

        if (!getInstance().pendingLoads.contains(key)) {
            getInstance().pendingLoads.insert(key);
            getInstance().worker->enqueueImageLoad(card);
        }
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
