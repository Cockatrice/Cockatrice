#include "user_card_art_provider.h"

#include "../../../card_picture_loader/card_picture_loader.h"

#include <QPointer>
#include <libcockatrice/card/database/card_database_manager.h>

static QString makeKey(const QString &user, const QString &card)
{
    return user + u'|' + card;
}

UserCardArtProvider::UserCardArtProvider(QObject *parent) : QObject(parent)
{
    dbReady = (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok);

    if (!dbReady) {
        connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
                &UserCardArtProvider::onDatabaseReady);
    }
}

void UserCardArtProvider::onDatabaseReady()
{
    dbReady = true;
    processQueue();
}

const QMap<QString, QPixmap> &UserCardArtProvider::cache() const
{
    return cardArtCache;
}

void UserCardArtProvider::requestCardArt(const QString &userName, const QString &cardName)
{
    if (cardName.isEmpty()) {
        return;
    }

    const QString key = makeKey(userName, cardName);

    if (cardArtCache.contains(key) || pending.contains(key)) {
        return;
    }

    pending.insert(key);
    queue.enqueue(key);

    processQueue();
}

QPixmap UserCardArtProvider::cropCardArt(const QPixmap &fullRes)
{
    const QSize sz = fullRes.size();
    const int marginX = sz.width() * 0.07;
    const int topMargin = sz.height() * 0.11;
    const int bottomMargin = sz.height() * 0.45;

    const QRect foilRect(marginX, topMargin, sz.width() - 2 * marginX, sz.height() - topMargin - bottomMargin);

    return fullRes.copy(foilRect.intersected(fullRes.rect()));
}

void UserCardArtProvider::insertIntoCache(const QString &key, const QPixmap &pixmap)
{
    if (!cardArtCache.contains(key)) {
        cacheInsertionOrder.append(key);
        while (cacheInsertionOrder.size() > MaxCacheEntries) {
            const QString evicted = cacheInsertionOrder.takeFirst();
            cardArtCache.remove(evicted);
        }
    }
    cardArtCache.insert(key, pixmap);
}

void UserCardArtProvider::processQueue()
{
    if (!dbReady) {
        return;
    }

    if (queue.isEmpty()) {
        return;
    }

    const QString key = queue.dequeue();

    const QStringList parts = key.split(u'|');
    if (parts.size() != 2) {
        pending.remove(key);
        processQueue();
        return;
    }

    const QString userName = parts.at(0);
    const QString cardName = parts.at(1);

    ExactCard card = CardDatabaseManager::query()->getCard({cardName});

    if (!card) {
        pending.remove(key);
        processQueue();
        return;
    }

    QPixmap fullRes;
    CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));

    // If already available, store immediately
    if (!fullRes.isNull()) {
        insertIntoCache(key, cropCardArt(fullRes));
        pending.remove(key);

        emit cardArtUpdated(userName);
        processQueue();
        return;
    }

    // Otherwise wait for async load
    QPointer<UserCardArtProvider> self(this);

    connect(card.getCardPtr().data(), &CardInfo::pixmapUpdated, this, [self, key, userName, card]() mutable {
        if (!self) {
            return;
        }

        disconnect(card.getCardPtr().data(), &CardInfo::pixmapUpdated, self, nullptr);

        QPixmap fullRes;
        CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));

        if (!fullRes.isNull()) {
            self->insertIntoCache(key, self->cropCardArt(fullRes));
        } else {
            self->insertIntoCache(key, QPixmap());
        }

        self->pending.remove(key);

        emit self->cardArtUpdated(userName);
        self->processQueue();
    });
}