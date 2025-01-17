#include "picture_loader.h"

#include "../../../settings/cache_settings.h"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QMovie>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmapCache>
#include <QScreen>
#include <QThread>
#include <algorithm>
#include <utility>

// never cache more than 300 cards at once for a single deck
#define CACHED_CARD_PER_DECK_MAX 300

PictureLoader::PictureLoader() : QObject(nullptr)
{
    worker = new PictureLoaderWorker;
    connect(&SettingsCache::instance(), SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(&SettingsCache::instance(), SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));

    connect(worker, SIGNAL(imageLoaded(CardInfoPtr, const QImage &)), this,
            SLOT(imageLoaded(CardInfoPtr, const QImage &)));
}

PictureLoader::~PictureLoader()
{
    worker->deleteLater();
}

void PictureLoader::getCardBackPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(PictureLoaderLog) << "PictureLoader: cache fail for" << backCacheKey;
        pixmap = QPixmap("theme:cardback").scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void PictureLoader::getCardBackLoadingInProgressPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(PictureLoaderLog) << "PictureLoader: cache fail for" << backCacheKey;
        pixmap = QPixmap("theme:cardback").scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void PictureLoader::getCardBackLoadingFailedPixmap(QPixmap &pixmap, QSize size)
{
    QString backCacheKey = "_trice_card_back_" + QString::number(size.width()) + QString::number(size.height());
    if (!QPixmapCache::find(backCacheKey, &pixmap)) {
        qCDebug(PictureLoaderLog) << "PictureLoader: cache fail for" << backCacheKey;
        pixmap = QPixmap("theme:cardback").scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(backCacheKey, pixmap);
    }
}

void PictureLoader::getPixmap(QPixmap &pixmap, CardInfoPtr card, QSize size)
{
    if (card == nullptr) {
        return;
    }

    // search for an exact size copy of the picture in cache
    QString key = card->getPixmapCacheKey();
    QString sizeKey = key + QLatin1Char('_') + QString::number(size.width()) + QString::number(size.height());
    if (QPixmapCache::find(sizeKey, &pixmap))
        return;

    // load the image and create a copy of the correct size
    QPixmap bigPixmap;
    if (QPixmapCache::find(key, &bigPixmap)) {
        QScreen *screen = qApp->primaryScreen();
        qreal dpr = screen->devicePixelRatio();
        pixmap = bigPixmap.scaled(size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(dpr);
        QPixmapCache::insert(sizeKey, pixmap);
        return;
    }

    // add the card to the load queue
    getInstance().worker->enqueueImageLoad(card);
}

void PictureLoader::imageLoaded(CardInfoPtr card, const QImage &image)
{
    if (image.isNull()) {
        QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap());
    } else {
        if (card->getUpsideDownArt()) {
            QImage mirrorImage = image.mirrored(true, true);
            QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap::fromImage(mirrorImage));
        } else {
            QPixmapCache::insert(card->getPixmapCacheKey(), QPixmap::fromImage(image));
        }
    }

    card->emitPixmapUpdated();
}

void PictureLoader::clearPixmapCache(CardInfoPtr card)
{
    if (card) {
        QPixmapCache::remove(card->getPixmapCacheKey());
    }
}

void PictureLoader::clearPixmapCache()
{
    QPixmapCache::clear();
}

void PictureLoader::clearNetworkCache()
{
    getInstance().worker->clearNetworkCache();
}

void PictureLoader::cacheCardPixmaps(QList<CardInfoPtr> cards)
{
    QPixmap tmp;
    int max = qMin(cards.size(), CACHED_CARD_PER_DECK_MAX);
    for (int i = 0; i < max; ++i) {
        const CardInfoPtr &card = cards.at(i);
        if (!card) {
            continue;
        }

        QString key = card->getPixmapCacheKey();
        if (QPixmapCache::find(key, &tmp)) {
            continue;
        }

        getInstance().worker->enqueueImageLoad(card);
    }
}

void PictureLoader::picDownloadChanged()
{
    QPixmapCache::clear();
}

void PictureLoader::picsPathChanged()
{
    QPixmapCache::clear();
}
