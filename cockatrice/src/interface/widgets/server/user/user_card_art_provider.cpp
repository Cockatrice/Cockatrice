#include "user_card_art_provider.h"

#include "../../../card_picture_loader/card_picture_loader.h"
#include "../../cards/card_art_utils.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/card/printing/printing_info.h"

#include <QChar>
#include <QMetaObject>
#include <QPixmapCache>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QTimer>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/utility/card_ref.h>

QString UserCardArtProvider::cacheKey(const QString &userName, const QString &cardName, const QString &providerId)
{
    // Length prefixes distinguish identities even when a component contains '|'.
    return QString::number(userName.size()) + u':' + userName + QString::number(cardName.size()) + u':' + cardName +
           providerId;
}

UserCardArtProvider::UserCardArtProvider(QObject *parent) : QObject(parent)
{
    auto *database = CardDatabaseManager::getInstance();
    dbReady = (database->getLoadStatus() == LoadStatus::Ok);
    connect(database, &CardDatabase::cardDatabaseReset, this, &UserCardArtProvider::onDatabaseReady);
}

void UserCardArtProvider::onDatabaseReady()
{
    // Connections capture exact printings from the old snapshot. Invalidate their
    // ownership before rebuilding; late completions must not populate the new cache.
    for (auto *context : activeRequests) {
        context->deleteLater();
    }
    activeRequests.clear();
    cardArtCache.clear();
    cacheInsertionOrder.clear();
    queue.clear();
    for (auto it = requests.cbegin(); it != requests.cend(); ++it) {
        queue.enqueue(it.key());
    }
    dbReady = (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok);
    const auto users = currentKeys.keys();
    QPointer<UserCardArtProvider> self(this);
    for (const auto &user : users) {
        emit cardArtUpdated(user);
        if (!self) {
            return;
        }
    }
    processQueue();
}

const QMap<QString, QPixmap> &UserCardArtProvider::cache() const
{
    return cardArtCache;
}

void UserCardArtProvider::requestCardArt(const QString &userName, const QString &cardName, const QString &providerId)
{
    const QString key = cacheKey(userName, cardName, providerId);
    const QString previousKey = currentKeys.value(userName);
    if (previousKey == key) {
        return;
    }
    if (!previousKey.isEmpty()) {
        forgetRequest(previousKey);
    }
    if (cardName.isEmpty()) {
        emit cardArtUpdated(userName);
        return;
    }

    currentKeys.insert(userName, key);
    requests.insert(key, {userName, cardName, providerId});
    queue.enqueue(key);
    processQueue();
}

QPixmap UserCardArtProvider::cropCardArt(const QPixmap &fullRes, const ExactCard &card)
{
    QPixmap source = fullRes;

    // Sideways-layout cards (plane, siege/battle, split) store their landscape
    // artwork rotated 90° inside a portrait frame. Rotate it upright first so
    // the crop below lands on the horizontal art, mirroring the way
    // CardInfoPictureWidget displays these cards.
    const bool landscape = card.getInfo().getUiAttributes().landscapeOrientation;
    source = CardArtUtils::rotateSidewaysLayoutArt(source, card);

    const QSize sz = source.size();
    const int marginX = sz.width() * 0.07;
    const int topMargin = landscape ? sz.height() * 0.05 : sz.height() * 0.11;
    const int bottomMargin = landscape ? sz.height() * 0.42 : sz.height() * 0.45;

    const QRect artRect(marginX, topMargin, sz.width() - 2 * marginX, sz.height() - topMargin - bottomMargin);

    return source.copy(artRect.intersected(source.rect()));
}

void UserCardArtProvider::insertIntoCache(const QString &key, const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return;
    }

    if (!cardArtCache.contains(key)) {
        cacheInsertionOrder.append(key);
        while (cacheInsertionOrder.size() > MaxCacheEntries) {
            const QString evicted = cacheInsertionOrder.takeFirst();
            cardArtCache.remove(evicted);
            currentKeys.remove(requests.value(evicted).userName);
            requests.remove(evicted);
        }
    }
    cardArtCache.insert(key, pixmap);
}

void UserCardArtProvider::forgetRequest(const QString &key)
{
    if (auto *context = activeRequests.take(key)) {
        context->deleteLater();
    }
    currentKeys.remove(requests.value(key).userName);
    requests.remove(key);
    cardArtCache.remove(key);
    cacheInsertionOrder.removeAll(key);
}

void UserCardArtProvider::finishRequest(const QString &key, QObject *context, const QPixmap &pixmap)
{
    if (activeRequests.value(key) != context) {
        return;
    }
    const QString userName = requests.value(key).userName;
    activeRequests.remove(key);
    context->deleteLater();
    if (pixmap.isNull()) {
        // A cached loader failure may not emit another pixmapUpdated signal.
        // Release the request so a later hover/refresh can try again.
        currentKeys.remove(userName);
        requests.remove(key);
    } else {
        insertIntoCache(key, pixmap);
    }
    emit cardArtUpdated(userName);
}

void UserCardArtProvider::processQueue()
{
    dbReady = (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok);
    if (!dbReady || processingQueue) {
        return;
    }

    processingQueue = true;
    QPointer<UserCardArtProvider> self(this);
    while (dbReady && !queue.isEmpty()) {
        const QString key = queue.dequeue();
        if (!requests.contains(key) || activeRequests.contains(key) || cardArtCache.contains(key)) {
            continue;
        }
        const Request request = requests.value(key);
        const ExactCard card = CardDatabaseManager::query()->getCard({request.cardName, request.providerId});
        if (!card) {
            forgetRequest(key);
            continue;
        }

        auto *context = new QObject(this);
        activeRequests.insert(key, context);
        // Connect before requesting: a synchronous completion must not be lost.
        connect(card.getCardPtr().data(), &CardInfo::pixmapUpdated, context,
                [this, key, card, context](const PrintingInfo &printing) {
                    // Other printings share CardInfo and emit the same signal.
                    if (activeRequests.value(key) != context || !(printing == card.getPrinting())) {
                        return;
                    }
                    QPixmap fullRes;
                    CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));
                    finishRequest(key, context, fullRes.isNull() ? QPixmap{} : cropCardArt(fullRes, card));
                });

        auto *deadline = new QTimer(context);
        deadline->setSingleShot(true);
        connect(deadline, &QTimer::timeout, context, [this, key, context] { finishRequest(key, context, {}); });
        deadline->start(30000);

        QPixmap fullRes;
        CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));
        if (!self) {
            return;
        }
        if (activeRequests.value(key) != context) {
            continue;
        }
        // The loader suppresses retries for a recently failed image. Such a
        // cached null result will not emit a completion, so release it now.
        QPixmap cached;
        if (fullRes.isNull() && QPixmapCache::find(card.getPixmapCacheKey(), &cached) && cached.isNull()) {
            forgetRequest(key);
            continue;
        }
        if (!fullRes.isNull()) {
            finishRequest(key, context, cropCardArt(fullRes, card));
            if (!self) {
                return;
            }
        }
        // Start the next banner independently. A failed or suppressed picture
        // request must not hold up the rest of the users queued before DB load.
    }
    processingQueue = false;
}
