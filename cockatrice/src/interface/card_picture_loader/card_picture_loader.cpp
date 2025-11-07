#include "card_picture_loader.h"

#include "../../client/settings/cache_settings.h"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QMainWindow>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmapCache>
#include <QScreen>
#include <QStatusBar>
#include <QThread>
#include <algorithm>
#include <utility>

// never cache more than 300 cards at once for a single deck
#define CACHED_CARD_PER_DECK_MAX 300

CardPictureLoader::CardPictureLoader() : QObject(nullptr)
{
    worker = new CardPictureLoaderWorker;
    connect(&SettingsCache::instance(), &SettingsCache::picsPathChanged, this, &CardPictureLoader::picsPathChanged);
    connect(&SettingsCache::instance(), &SettingsCache::picDownloadChanged, this,
            &CardPictureLoader::picDownloadChanged);

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
    worker->deleteLater();
}

void CardPictureLoader::getCardBackPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + "x" + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(CardPictureLoaderLog) << "PictureLoader: cache miss for" << backCacheKey;
        QPixmap tmpPixmap("theme:cardback");

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
        QPixmap tmpPixmap("theme:cardback");

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
        QPixmap tmpPixmap("theme:cardback");

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
            qCDebug(CardPictureLoaderLog) << "Cached pixmap for key" << key << "is NULL!";
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
    if (image.isNull()) {
        qCDebug(CardPictureLoaderLog) << "Caching NULL pixmap for" << card.getName();
        QPixmapCache::insert(card.getPixmapCacheKey(), QPixmap());
    } else {
        if (card.getInfo().getUpsideDownArt()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))
            QImage mirrorImage = image.flipped(Qt::Horizontal | Qt::Vertical);
#else
            QImage mirrorImage = image.mirrored(true, true);
#endif
            QPixmapCache::insert(card.getPixmapCacheKey(), QPixmap::fromImage(mirrorImage));
        } else {
            QPixmapCache::insert(card.getPixmapCacheKey(), QPixmap::fromImage(image));
        }
    }

    // imageLoaded should only be reached if the exactCard isn't already in cache.
    // (plus there's a deduplication mechanism in CardPictureLoaderWorker)
    // It should be safe to connect the CardInfo here without worrying about redundant connections.
    connect(card.getCardPtr().data(), &QObject::destroyed, this,
            [cacheKey = card.getPixmapCacheKey()] { QPixmapCache::remove(cacheKey); });

    card.emitPixmapUpdated();
}

void CardPictureLoader::clearPixmapCache()
{
    QPixmapCache::clear();
}

void CardPictureLoader::clearNetworkCache()
{
    getInstance().worker->clearNetworkCache();
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

bool CardPictureLoader::hasCustomArt()
{
    auto picsPath = SettingsCache::instance().getPicsPath();
    QDirIterator it(picsPath, QDir::Dirs | QDir::NoDotAndDotDot);

    // Check if there is at least one non-directory file in the pics path, other
    // than in the "downloadedPics" subdirectory.
    while (it.hasNext()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
        QFileInfo dir(it.nextFileInfo());
#else
        // nextFileInfo() is only available in Qt 6.3+, for previous versions, we build
        // the QFileInfo from a QString which requires more system calls.
        QFileInfo dir(it.next());
#endif

        if (it.fileName() == "downloadedPics")
            continue;

        QDirIterator subIt(it.filePath(), QDir::Files, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        if (subIt.hasNext()) {
            return true;
        }
    }

    return false;
}
